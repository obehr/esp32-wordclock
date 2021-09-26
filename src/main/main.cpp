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
#include <esp_wifi.h>
#include <esp_netif.h>
#include "esp_log.h"

#include "wifi_manager.h"
#include "http_app.h"

#include "display.cpp"
#include "config.cpp"

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
Display my_display(26, 1, 0);
//ESP_LOGI(TAG, "created display");
bool display = false;

extern "C" {
  void app_main();
}



/**
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information, should not be ever started on production code! This is an example on how you can integrate your code with wifi-manager
 */






/**
 * @brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event.
 */
void cb_connection_ok(void *pvParameter){
	ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

	/* transform IP to human readable string */
	char str_ip[16];
	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

	ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "fritz.box");
  sntp_init();
  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);
}

void cb_received_config(void *pvParameter){
  ESP_LOGI(TAG, "callback received_config");

	my_config* new_config = (my_config*)pvParameter;
  ESP_LOGI(TAG, "casted to my_config");
  int16_t current_hour = new_config->hour;
  ESP_LOGI(TAG, "got hour from config %d", current_hour);
}

static void loop_time(void *pvParameters)
{
  letzteMinute=-1;
  letzteStunde=-1;
  int16_t config_hour = -1;
  while(true)
  {
    
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);

    for(int tempHour=0; tempHour<12; tempHour++)
    {
      ESP_LOGI(TAG, "call get_hour");
      int16_t current_config_hour = get_hour();
      if(current_config_hour!=-1)
      { ESP_LOGI(TAG, "got hour from config %d", current_config_hour); }
      if(current_config_hour != config_hour)
      {
        config_hour = current_config_hour;
        tempHour = config_hour;
      }
      for(int tempMinute=0; tempMinute<11; tempMinute++)
      {
        
        display = true;
        my_display.stelle_zeit(letzteStunde, letzteMinute, tempHour, tempMinute);
        my_display.bearbeiteListe(2);
        display = false;
        letzteStunde=tempHour;
        letzteMinute=tempMinute;
        vTaskDelay( pdMS_TO_TICKS(1000) );
      }
    }
  }
}



void app_main() {
  
  
  ESP_LOGI(TAG, " entering app main, call add leds\n");
  // the WS2811 family uses the RMT driver
  //FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS);
  //FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS);

  // this is a good test because it uses the GPIO ports, these are 4 wire not 3 wire
  //FastLED.addLeds<APA102, 13, 15>(leds, NUM_LEDS);
  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;

  time(&now);
  setenv("TZ", "UTC-2", 1);
  tzset();

  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);
/* start the wifi manager */
	wifi_manager_start();
	/* register a callback as an example to how you can integrate your code with the wifi manager */
	wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);
	wifi_manager_set_callback(WM_RECEIVED_CONFIG, &cb_received_config);

	/* your code should go here. Here we simply create a task on core 2 that monitors free heap memory */
	//xTaskCreatePinnedToCore(&monitoring_task, "monitoring_task", 2048, NULL, 1, NULL, 1);

  // I have a 2A power supply, although it's 12v



  //xTaskCreatePinnedToCore(&blinkLeds_simple, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&fastfade, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&blinkWithFx_allpatterns, "blinkLeds", 4000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&loop_time, "loop time", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&loop_display, "loop time", 4000, NULL, 5, NULL, 0);

  //xTaskCreatePinnedToCore(&blinkLeds_chase, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&blinkLeds_chase2, "blinkLeds", 4000, NULL, 5, NULL, 0);
}
