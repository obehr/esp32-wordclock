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

static const char TAG[] = "wordclock";


const char *passwort1 = "SOURSEVE";
const char *passwort2 = "SVNEHTNV";
const char *passwort3 = "OSVNRNEV";
const char *apName1 = "WordclockV1";
const char *apName2 = "WordclockV2";
const char *apName3 = "WordclockV3";

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

extern "C" {
  void app_main();
}

time_t now;


/**
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information, should not be ever started on production code! This is an example on how you can integrate your code with wifi-manager
 */






/**
 * @brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event.
 */


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
  
  int16_t current_hour = current_config->hour;
  ESP_LOGI(TAG, "got hour from config %d", current_hour);

  if(current_config->time_changed)
  {
    ESP_LOGI(TAG, "got new time config: %d, %d, %d, %d", current_config->hour ,current_config->minute, current_config->use_ntp, current_config->time_offset);
    
    if(current_config->use_ntp)
    {
      ESP_LOGI(TAG, "Activate NTP");
      sntp_setservername(0, "fritz.box");
      //sntp_setoperatingmode(SNTP_OPMODE_POLL);
      switch(current_config->time_offset) {
        case -4: setenv("TZ", "UTC-4", 1); break;
        case -3: setenv("TZ", "UTC-3", 1); break;
        case -2: setenv("TZ", "UTC-2", 1); break;
        case -1: setenv("TZ", "UTC-1", 1); break;
        case 0: setenv("TZ", "UTC0", 1); break;
        case 1: setenv("TZ", "UTC1", 1); break;
        case 2: setenv("TZ", "UTC2", 1); break;
        case 3: setenv("TZ", "UTC3", 1); break;
        case 4: setenv("TZ", "UTC4", 1); break;
        default: ESP_LOGI(TAG, "unhandled offset"); break;
      }
      sntp_init();
    }
    else
    {
      ESP_LOGI(TAG, "Deactivate NTP");
      sntp_setservername(0, "localhost");
      //sntp_setoperatingmode(SNTP_OPMODE_LISTENONLY);

      char strftime_buf[64];
      struct tm timeinfo;
      time(&now);
      localtime_r(&now, &timeinfo);
      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      ESP_LOGI(TAG, "The time before setting it: %s", strftime_buf);
      

      struct timeval tv;
      //add hours in seconds and minutes in seconds to midnight of Octover 8th 2021
      tv.tv_sec = 1633644000+3600*current_config->hour+60*current_config->minute;
      settimeofday(&tv, NULL);

      time(&now);
      localtime_r(&now, &timeinfo);
      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      ESP_LOGI(TAG, "The time after setting it: %s", strftime_buf);
    }



    //reset boolean to false
    current_config->time_changed = false;
  }
  if(current_config->color_changed)
  {
    ESP_LOGI(TAG, "got new colors: %d, %d, %d, %d", current_config->color_its_oclock,current_config->color_minutes,current_config->color_past_to,current_config->color_hours);
    my_display.setze_farben(current_config->color_its_oclock,current_config->color_minutes,current_config->color_past_to,current_config->color_hours, current_config->brightness, current_config->saturation);
    //reset boolean to false
    current_config->color_changed = false;
  }
  
}


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

void initialize_sntp()
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "fritz.box");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    sntp_init();
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

void cb_connection_ok(void *pvParameter){
	ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

	/* transform IP to human readable string */
	char str_ip[16];
	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

	ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);

  time_t now;
  struct tm timeinfo;
  char strftime_buf[64];

  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The time before sntp setup is: %s", strftime_buf);

  ESP_LOGI(TAG, "Get time over NTP");
  initialize_sntp();
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

/*bool synched=false;
  while(!synched)
  {
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    if(wait_for_sync())
    {
      ESP_LOGI(TAG, "Now it is in sync");
      
      localtime_r(&now, &timeinfo);
      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      ESP_LOGI(TAG, "The current date/time in New York is: %s", strftime_buf);
      synched=true;
    }
    else
    {
      localtime_r(&now, &timeinfo);
      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      ESP_LOGI(TAG, "Finally failed");
      if (timeinfo.tm_year < (2016 - 1900)) {
          ESP_LOGI(TAG, "The time is still out of range: %s", strftime_buf);
      }
      else {
          ESP_LOGI(TAG, "Time is in range, though: %s", strftime_buf);
          synched = true;
      }
    }
  }*/

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
  current_config = get_default_config();
  int16_t current_hour = current_config->hour;
  ESP_LOGI(TAG, "got hour from config %d", current_hour);
  

  setenv("TZ", "UTC-1", 1);
  tzset();

  

/* start the wifi manager */
	wifi_manager_start();
	/* register a callback as an example to how you can integrate your code with the wifi manager */
	wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);
	wifi_manager_set_callback(WM_RECEIVED_CONFIG, &cb_received_config);

	/* your code should go here. Here we simply create a task on core 2 that monitors free heap memory */
	//xTaskCreatePinnedToCore(&monitoring_task, "monitoring_task", 2048, NULL, 1, NULL, 1);




  //xTaskCreatePinnedToCore(&blinkLeds_simple, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&fastfade, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&blinkWithFx_allpatterns, "blinkLeds", 4000, NULL, 5, NULL, 0);
  my_display.setze_farben(current_config->color_its_oclock,current_config->color_minutes,current_config->color_past_to,current_config->color_hours, current_config->brightness, current_config->saturation);

  
  xTaskCreatePinnedToCore(&loop_time, "loop time", 4000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&start_loop_display, "start loop display", 4000, NULL, ( 1UL | portPRIVILEGE_BIT ), NULL, 0);
  
  //xTaskCreatePinnedToCore(&loop_display, "loop time", 4000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&print_time, "print time", 4000, NULL, 5, NULL, 0);

  //xTaskCreatePinnedToCore(&blinkLeds_chase, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&blinkLeds_chase2, "blinkLeds", 4000, NULL, 5, NULL, 0);
}
