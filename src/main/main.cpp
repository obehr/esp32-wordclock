/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_sntp.h"
#include <string.h>
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"

#include <esp_wifi.h>
#include <esp_netif.h>

#include "wifi_manager.h"
#include "http_app.h"

#include "display.cpp"
#include "config.cpp"
//#include "sntp.cpp"
#include "nvs_flash.h"
#include "nvs.h"

static const char TAG[] = "wordclock";


const char *passwort1 = "SOURSEVE";
const char *passwort2 = "SVNEHTNV";
const char *passwort3 = "OSVNRNEV";
const char *apName1 = "WordclockV1";
const char *apName2 = "WordclockV2";
const char *apName3 = "WordclockV3";

bool ntp_initialized=false;
int aktuelleMinute;
int aktuelleStunde;
int letzteMinute;
int letzteStunde;

int16_t currentMinute;
int16_t currentHour;

int16_t lastMinute=-1;
//ESP_LOGI(TAG, "create display");
Display my_display{};
my_config* current_config;
//ESP_LOGI(TAG, "created display");
bool display = false;
struct tm manual_time;

extern "C" {
  void app_main();
}

time_t now;


/**
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information, should not be ever started on production code! This is an example on how you can integrate your code with wifi-manager
 */



void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
    if(sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
    {
        ESP_LOGI(TAG, "Time was synched (again)");
        
        struct tm timeinfo;
        char strftime_buf[64];
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The time after verified sync: %s", strftime_buf);
    }
    else
    {
        ESP_LOGI(TAG, "Time sync process is ongoing or was reset");
    }
}


/**
 * @brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event.
 */

void set_timezone_offset(int16_t offset)
{
  switch(offset) {
    case 0: setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); tzset(); break;
    case 1: setenv("TZ", "UTC+3", 1); tzset(); break;
    case 2: setenv("TZ", "UTC+2", 1); tzset(); break;
    case 3: setenv("TZ", "UTC+1", 1); tzset(); break;
    case 4: setenv("TZ", "UTC0", 1); tzset(); break;
    case 5: setenv("TZ", "UTC-1", 1); tzset(); break;
    case 6: setenv("TZ", "UTC-2", 1); tzset(); break;
    case 7: setenv("TZ", "UTC-3", 1); tzset(); break;
    default: ESP_LOGI(TAG, "unhandled offset"); break;
  }
}

void set_ntp_server()
{
  ESP_LOGI(TAG, "Activate NTP");
  if(!ntp_initialized)
  {
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    ntp_initialized = true;
  }
  sntp_setservername(0, "fritz.box");
  sntp_setservername(1, "de.pool.ntp.org");

  sntp_init();
}

void deactivate_ntp()
{
  ESP_LOGI(TAG, "Deactivate NTP");
  sntp_setservername(0, "localhost");
}


void get_time_from_config()
{
  if(current_config->year > 1900 && current_config->year < 2038)
  { manual_time.tm_year=current_config->year-1900; }
  else
  { manual_time.tm_year = 0; }

  if(current_config->month > 0 && current_config->month < 13)
  { manual_time.tm_mon=current_config->month-1; }
  else
  { manual_time.tm_mon = 0; }

  if(current_config->day > 0 && current_config->day < 32)
  { manual_time.tm_mday=current_config->day; }
  else
  { manual_time.tm_mday = 0; }

  if(current_config->hour < 24)
  { manual_time.tm_hour=current_config->hour; }
  else
  { manual_time.tm_hour = 0; }

  if(current_config->minute < 60)
  { manual_time.tm_min = current_config->minute; }
  else
  { manual_time.tm_min = 0; }
}


void set_manual_time()
{
  long manual_timestamp = mktime(&manual_time);
  if(manual_timestamp == -1)
  {
    ESP_LOGI(TAG, "Convert date/time to unix timestamp failed");
    return;
  }
  else
  { ESP_LOGI(TAG, "Date/time converted to unix timestamp: %ld", manual_timestamp); }
  
  //strftime
  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &manual_time);

  long unixts = time(&now);
  ESP_LOGI(TAG, "Unix timestamp before update: %ld", unixts);
  
  struct timeval tv;
  
  tv.tv_sec = 0;
  ESP_LOGI(TAG, "Setting timeofday to tv_sec = 0");
  settimeofday(&tv, NULL);
  long unixts_zero = time(&now);
  ESP_LOGI(TAG, "Unix timestamp at tv_sec = 0 is: %ld", unixts_zero);
  
  if(manual_timestamp < unixts_zero)
  {
    ESP_LOGI(TAG, "Error: timestamp offset at tv_sec = 0 is bigger than target timestamp");
    return;
  }

  tv.tv_sec = manual_timestamp - unixts_zero;
  ESP_LOGI(TAG, "Setting tv_sec to manual unix timestamp minus unix timestamp at tv_sec = 0: %ld", tv.tv_sec);
  settimeofday(&tv, NULL);
  unixts = time(&now);
  ESP_LOGI(TAG, "Verify unix timestamp after setting: %ld", unixts);
  
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The time after setting it: %s", strftime_buf);
}

void set_time_config(bool use_ntp, int16_t offset)
{
  ESP_LOGI(TAG, "Set time config. Use NTP %d, Time Offset %d ...", use_ntp, offset);
  ESP_LOGI(TAG, "... manual date and time: %d-%d-%d %d:%d", manual_time.tm_year, manual_time.tm_mon, manual_time.tm_mday, manual_time.tm_hour, manual_time.tm_min);
  
  set_timezone_offset(offset);

  if(use_ntp)
  {
    set_ntp_server();
  }
  else
  { 
    deactivate_ntp();
    set_manual_time(); 
  }
}


void cb_received_config(void *pvParameter){
  ESP_LOGI(TAG, "callback received_config");
  current_config = (my_config*)pvParameter;
  ESP_LOGI(TAG, "casted to my_config");
	

  if(!current_config->config_changed)
  {
    ESP_LOGI(TAG, "Config did not change");
    return;
  }
  //reset boolean to false
  current_config->config_changed = false;

  if(current_config->time_changed)
  {
    get_time_from_config();    
    set_time_config(current_config->use_ntp, current_config->time_offset);
    
    //reset boolean to false
    current_config->time_changed = false;
  }
  if(current_config->color_changed)
  {
    ESP_LOGI(TAG, "got new colors: %d, %d, %d, %d", current_config->color_its_oclock,current_config->color_minutes,current_config->color_past_to,current_config->color_hours);
    my_display.setze_farben(current_config->color_its_oclock,current_config->color_minutes,current_config->color_past_to,current_config->color_hours);
    //reset boolean to false
    current_config->color_changed = false;
  }
  if(current_config->brightness_changed)
  {
    ESP_LOGI(TAG, "got new brightness: %d", current_config->brightness);
    my_display.setze_helligkeit(current_config->brightness);
    current_config->brightness_changed = false;
  }
  if(current_config->saturation_changed)
  {
    ESP_LOGI(TAG, "got new saturation: %d", current_config->saturation);
    my_display.setze_saettigung(current_config->saturation);
    current_config->saturation_changed = false;
  }
}

void cb_display_off(void *pvParameter){
  ESP_LOGI(TAG, "callback display off");
  my_display.mode = 6;
  set_display_status(false);
}

void cb_display_on(void *pvParameter){
  ESP_LOGI(TAG, "callback display on");
  my_display.mode = 7;
  set_display_status(true);
}

void cb_strip_off(void *pvParameter){
  ESP_LOGI(TAG, "callback strip off");
  my_display.mode = 4;
  set_stripe_status(false);
}

void cb_strip_on(void *pvParameter){
  ESP_LOGI(TAG, "callback strip on");
  my_display.mode = 5;
  set_stripe_status(true);
}

bool wait_for_sync()
{
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if(sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
    {
        ESP_LOGI(TAG, "Time was retrieved");
        return true;
    }
    else
    {
        ESP_LOGI(TAG, "Time sync process is ongoing or was reset");
        return false;
    }
    
}

/*void print_ip_address(char[] *ip)
{
 int wortIP[2][2] =
        { 4, 1, 4, 2 };
    cf.reiheLedsInListe (wortIP, 2, 3, Wordlayout);
    cf.bearbeiteListe (5);
    delay (2000);
    cf.bearbeiteListe (6);

    for (int i = startOktett; i <= endeOktett; i++)
    {
        Serial.println ();
        uint8_t oktettInt = ip[i];
        Serial.print (oktettInt);
        char oktettChar[3];
        itoa (oktettInt, oktettChar, 10);
        for (int j = 0; j < 3; j++)
        {
            Serial.print (" -> ");
            int ziffer = oktettChar[j] - '0';
            Serial.print (ziffer);
            if (ziffer > 0 && ziffer <= 9)
            {
                cf.reiheStundenInListe (ziffer, 3, Wordlayout);
                cf.bearbeiteListe (4);
                delay (500);
                cf.bearbeiteListe (6);
            }
            else if (ziffer == 0)
            {
                cf.reiheStundenInListe (10, 3, Wordlayout);
                cf.bearbeiteListe (4);
                delay (500);
                cf.bearbeiteListe (6);
            }
            else
            {
                break;  //Ende der Zahl
            }
        }
    }
}

  int count_dot = 0;
  for(int i=0; i<16; i++)
  {
	  ESP_LOGI(TAG, "printing digit %d!", ip[i]);
  }
}*/

void cb_connection_ok(void *pvParameter){
	ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

	/* transform IP to human readable string */
	char str_ip[16];
	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);
  
	ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);
  //print_ip_address(&str_ip);
  time_t now;
  struct tm timeinfo;
  char strftime_buf[64];

  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The time before sntp setup is: %s", strftime_buf);
}

static void print_time(void *pvParameters)
{ 
  while(true)
  {
    char strftime_buf[64];
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);
    vTaskDelay( pdMS_TO_TICKS(100000) );
  }
}


static void loop_time(void *pvParameters)
{
  char strftime_buf[64];
  struct tm timeinfo;
  
  //wait until time is set
  while(true)
  {
    time(&now);
    localtime_r(&now, &timeinfo);
    if(timeinfo.tm_year > 120)
    { break; }
    ESP_LOGI(TAG, "The time is not yet set %d", timeinfo.tm_year);
    vTaskDelay( pdMS_TO_TICKS(10000) );
  }

  uint16_t letzte_fuenfminute =- 1;
  uint16_t letzte_stunde =- 1;
  uint16_t aktuelle_fuenfminute;
  uint16_t aktuelle_stunde;

  //eternal loop
  while(true)
  {
    char strftime_buf[64];
    struct tm timeinfo;
    
    //wait until display time changed
    while(true)
    {
      time(&now);
      localtime_r(&now, &timeinfo);

      aktuelle_stunde = (timeinfo.tm_min < 33) ? timeinfo.tm_hour % 12 : (timeinfo.tm_hour + 1) % 12;
      aktuelle_fuenfminute = (timeinfo.tm_min + 2) % 60 / 5;

      if(aktuelle_fuenfminute != letzte_fuenfminute || aktuelle_stunde != letzte_stunde)
      {
        break;
      }
      //ESP_LOGI(TAG, "The display time has not changed %d : %d", aktuelle_stunde, aktuelle_fuenfminute);
      vTaskDelay( pdMS_TO_TICKS(10000) );
    }
    
    ESP_LOGI(TAG, "set disply time to %d : %d", aktuelle_stunde, aktuelle_fuenfminute);
    my_display.mode = 0; //disable animation
    while(my_display.mode == 0)
    { 
      vTaskDelay( pdMS_TO_TICKS(50) ); //wait for animation to end, display will set mode to -1
    }
    
    my_display.stelle_zeit(letzte_stunde, letzte_fuenfminute, aktuelle_stunde, aktuelle_fuenfminute);
    
    my_display.mode = 2;
    while(my_display.mode == 2)
    { 
      vTaskDelay( pdMS_TO_TICKS(50) ); //wait for setting to end, display will set mode to -1
    }

    my_display.mode = 3;
    
    letzte_stunde = aktuelle_stunde;
    letzte_fuenfminute = aktuelle_fuenfminute;
  }
}

static void start_loop_display(void *pvParameters)
{
  ESP_LOGI(TAG, "Call my_display.loop_display");
  my_display.loop_display();
}

void app_main() {
  
  
  ESP_LOGI(TAG, " entering app main, call add leds\n");
  // the WS2811 family uses the RMT driver
  //FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS);
  //FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS);

  // this is a good test because it uses the GPIO ports, these are 4 wire not 3 wire
  //FastLED.addLeds<APA102, 13, 15>(leds, NUM_LEDS);
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
  }
  ESP_ERROR_CHECK( err );
  current_config = get_initial_config();
  get_time_from_config();
  set_time_config(current_config->use_ntp, current_config->time_offset);

  wifi_manager_init();
/* start the wifi manager */
	wifi_manager_start();
	/* register a callback as an example to how you can integrate your code with the wifi manager */
	wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);
	wifi_manager_set_callback(WM_RECEIVED_CONFIG, &cb_received_config);
  wifi_manager_set_callback(WM_DISPLAY_OFF, &cb_display_off);
  wifi_manager_set_callback(WM_DISPLAY_ON, &cb_display_on);
  wifi_manager_set_callback(WM_STRIP_OFF, &cb_strip_off);
  wifi_manager_set_callback(WM_STRIP_ON, &cb_strip_on);

	/* your code should go here. Here we simply create a task on core 2 that monitors free heap memory */
	//xTaskCreatePinnedToCore(&monitoring_task, "monitoring_task", 2048, NULL, 1, NULL, 1);




  //xTaskCreatePinnedToCore(&blinkLeds_simple, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&fastfade, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&blinkWithFx_allpatterns, "blinkLeds", 4000, NULL, 5, NULL, 0);
  my_display.setze_farben(current_config->color_its_oclock,current_config->color_minutes,current_config->color_past_to,current_config->color_hours);
  my_display.setze_helligkeit(current_config->brightness);
  my_display.setze_saettigung(current_config->saturation);
  my_display.strip_on = get_stripe_status();
  my_display.display_on = get_display_status();
  
  
  xTaskCreatePinnedToCore(&loop_time, "loop time", 4000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&start_loop_display, "start loop display", 4000, NULL, ( 1UL | portPRIVILEGE_BIT ), NULL, 0);
  
  my_display.mode = 5;
  //xTaskCreatePinnedToCore(&loop_display, "loop time", 4000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&print_time, "print time", 4000, NULL, 5, NULL, 0);

  //xTaskCreatePinnedToCore(&blinkLeds_chase, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&blinkLeds_chase2, "blinkLeds", 4000, NULL, 5, NULL, 0);
}
