#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <string.h>
#include "cJSON.h"
#include <esp_log.h>
#include "esp_spi_flash.h"


static const char TAG3[] = "config";

typedef struct {
  int16_t hour;
  int16_t minute;
  bool use_ntp;
} my_config;

static my_config valid_config;

//static char config_raw[300];
static int16_t hour_config = -1;

int16_t variant_led_numbering;
int16_t variant_clockface;
int16_t orientation;

static int16_t get_hour()
{
  ESP_LOGI(TAG3, "get_hour");
  ESP_LOGI(TAG3, "get_hour called and returned %d", valid_config.hour);
  return valid_config.hour;
}

static void* get_config()
{
  ESP_LOGI(TAG3, "get_config called");
  return (void*)&valid_config;
}

static void save_config(char *config_raw, size_t length)
{
  cJSON *json = cJSON_ParseWithLength(config_raw, length);
  char *string = cJSON_Print(json);
  ESP_LOGI(TAG3, "parsed json %s", string);
  //cJSON *root = cJSON_Parse(content);
  //content[recv_size] = '\0';
  cJSON *hour_json = cJSON_GetObjectItemCaseSensitive(json, "hour");
  if(cJSON_IsNumber(hour_json))
  {
      ESP_LOGI(TAG3,"is a number");
  }
  else
  {
    ESP_LOGI(TAG3, "no number");
  }

  valid_config.hour = atoi(hour_json->valuestring);
  //hour_config = atoi(hour_json->valuestring);

  //char *value_string = hour_json->valuestring;
  ESP_LOGI(TAG3, "value is %s", hour_json->valuestring);
  ESP_LOGI(TAG3, "value is %d", valid_config.hour);

}

