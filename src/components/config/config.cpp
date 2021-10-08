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
  uint16_t hour;
  uint16_t minute;
  bool use_ntp;
  uint16_t color_its_oclock;
  uint16_t color_minutes;
  uint16_t color_past_to;
  uint16_t color_hours;
  uint16_t saturation;
  uint16_t brightness;
  bool time_changed;
  bool color_changed;
  bool config_changed;
} my_config;

static bool config_available = false;

static my_config valid_config;

static my_config default_config;

static void init_default_config()
{
  ESP_LOGI(TAG3, "Initialize default config");
  default_config.hour = 0;
  default_config.minute = 0;
  default_config.use_ntp = false;
  default_config.color_its_oclock = 50;
  default_config.color_minutes = 100;
  default_config.color_past_to = 150;
  default_config.color_hours = 200;
  default_config.saturation = 255;
  default_config.brightness = 100;
  default_config.time_changed = false;
  default_config.color_changed = false;
  default_config.config_changed = false;
}

static my_config* get_default_config()
{
  init_default_config();
  
  ESP_LOGI(TAG3, "Passing default config");
  return &default_config;
}

static void* get_config()
{
  if(config_available)
  {
    ESP_LOGI(TAG3, "Passing valid config");
    return (void*)&valid_config;
  }
  else
  {
    default_config.config_changed = false;
    ESP_LOGI(TAG3, "Config has not changed");
    return (void*)&default_config;
  }
}

static int16_t get_number(cJSON *json_object)
{
  if(strcmp(json_object->valuestring, "") != 0)
  {
    int16_t casted_object = atoi(json_object->valuestring);
    if(casted_object > 0)
    { 
      return casted_object;
    }
  }
  return -1;
}

static void save_config(char *config_raw, size_t length)
{
  /*
  {
    "ntpUse":"0",
    "hour":"22",
    "minute":"22",
    "wifiConnect":"0",
    "ssid":"",
    "password":"",
    "ntpServer":"",
    "c1":"0",
    "c2":"32",
    "c3":"64",
    "c4":"96",
    "sat":"167",
    "bri":"210"
  }
  */

  if(!config_available)
  {
    init_default_config();
    valid_config.hour = default_config.hour;
    valid_config.minute = default_config.minute;
    valid_config.use_ntp = default_config.use_ntp;
    valid_config.color_its_oclock = default_config.color_its_oclock;
    valid_config.color_minutes = default_config.color_minutes;
    valid_config.color_past_to = default_config.color_past_to;
    valid_config.color_hours = default_config.color_hours;
    valid_config.saturation = default_config.saturation;
    valid_config.brightness = default_config.brightness;
  }

  cJSON *json = cJSON_ParseWithLength(config_raw, length);
  char *string = cJSON_Print(json);
  ESP_LOGI(TAG3, "parsed json %s", string);
  //cJSON *root = cJSON_Parse(content);
  //content[recv_size] = '\0';

  bool time_changed = false;
  bool color_changed = false;

  int16_t value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "hour"));
  if(value_casted != -1 && value_casted < 24 && value_casted != valid_config.hour)
  {
    valid_config.hour = value_casted;
    time_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "minute"));
  if(value_casted != -1 && value_casted < 24 && value_casted != valid_config.minute)
  {
    valid_config.minute = value_casted;
    time_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "ntpUse"));
  if((value_casted == 1) != valid_config.use_ntp)
  {
    ESP_LOGI(TAG3, "casted ntp value %d", value_casted);
    valid_config.use_ntp = (value_casted==1);
    time_changed = true;
  }
  else
  {
    ESP_LOGI(TAG3, "unchanged ntp value %s", cJSON_GetObjectItemCaseSensitive(json, "ntpUse")->valuestring);
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c1"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != valid_config.color_its_oclock)
  {
    valid_config.color_its_oclock = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c2"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != valid_config.color_minutes)
  {
    valid_config.color_minutes = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c3"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != valid_config.color_past_to)
  {
    valid_config.color_past_to = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c4"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != valid_config.color_hours)
  {
    valid_config.color_hours = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "bri"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != valid_config.brightness)
  {
    valid_config.brightness = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "sat"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != valid_config.saturation)
  {
    valid_config.saturation = value_casted;
    color_changed = true;
  }


  valid_config.time_changed = time_changed;
  valid_config.color_changed = color_changed;
  valid_config.config_changed = color_changed || time_changed;
  config_available = true;
  /*
  uint16_t minute_casted = atoi(minute_json->valuestring);
  bool use_ntp_casted = (atoi(minute_json->valuestring) == 1);
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
  */
}

