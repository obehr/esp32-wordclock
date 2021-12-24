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
#include "nvs_flash.h"
#include "nvs.h"

static const char TAG3[] = "config";

typedef struct {
  uint16_t hour;
  uint16_t minute;
  bool use_ntp;
  int16_t time_offset;
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

static my_config active_config;

static int32_t restart_counter = 0;

static void init_config()
{
  ESP_LOGI(TAG3, "Read config from nvs");
  active_config.hour = 0;
  active_config.minute = 0;
  active_config.use_ntp = false;
  active_config.time_offset = -1;
  active_config.color_its_oclock = 50;
  active_config.color_minutes = 100;
  active_config.color_past_to = 150;
  active_config.color_hours = 200;
  active_config.saturation = 255;
  active_config.brightness = 100;
  active_config.time_changed = false;
  active_config.color_changed = false;
  active_config.config_changed = false;

  esp_err_t err;
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGI(TAG3, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    // Read
    
    err = nvs_get_u16(my_handle, "hour", &active_config.hour);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read hour from NVS %d", active_config.hour); }

    err = nvs_get_u16(my_handle, "minute", &active_config.minute);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read minute from NVS %d", active_config.minute); }

    uint8_t int_use_ntp;
    err = nvs_get_u8(my_handle, "use_ntp", &int_use_ntp);
    if(err == ESP_OK)
    { 
      active_config.use_ntp = (int_use_ntp==1);
      ESP_LOGI(TAG3, "Read use_ntp from NVS %d", active_config.use_ntp);
    }

    err = nvs_get_i16(my_handle, "time_offset", &active_config.time_offset);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read time_offset from NVS %d", active_config.time_offset); }

    err = nvs_get_u16(my_handle, "color_its_oclock", &active_config.color_its_oclock);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read color_its_oclock from NVS %d", active_config.color_its_oclock); }

    err = nvs_get_u16(my_handle, "color_minutes", &active_config.color_minutes);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read color_minutes from NVS %d", active_config.color_minutes); }

    err = nvs_get_u16(my_handle, "color_past_to", &active_config.color_past_to);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read color_past_to from NVS %d", active_config.color_past_to); }

    err = nvs_get_u16(my_handle, "color_hours", &active_config.color_hours);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read color_hours from NVS %d", active_config.color_hours); }   

    err = nvs_get_u16(my_handle, "saturation", &active_config.saturation);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read saturation from NVS %d", active_config.saturation); }

    err = nvs_get_u16(my_handle, "brightness", &active_config.brightness);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read brightness from NVS %d", active_config.brightness); }   
  }
  config_available = true;
}

static my_config* get_initial_config()
{
  init_config();
  
  ESP_LOGI(TAG3, "Passing active config");
  return &active_config;
}

/*static void* get_config()
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
}*/

static void* get_config()
{
  if(!config_available)
  { init_config(); }

  ESP_LOGI(TAG3, "Passing active config");
  return (void*)&active_config;
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
    "ntpServer":"",
    "timeOffset":"",
    "c1":"0",
    "c2":"32",
    "c3":"64",
    "c4":"96",
    "sat":"167",
    "bri":"210"
  }
  */

  if(!config_available)
  { init_config(); }

  cJSON *json = cJSON_ParseWithLength(config_raw, length);
  char *string = cJSON_Print(json);
  ESP_LOGI(TAG3, "parsed json %s", string);
  //cJSON *root = cJSON_Parse(content);
  //content[recv_size] = '\0';

  bool time_changed = false;
  bool color_changed = false;

  int16_t value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "hour"));
  if(value_casted != -1 && value_casted < 24 && value_casted != active_config.hour)
  {
    active_config.hour = value_casted;
    time_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "minute"));
  if(value_casted != -1 && value_casted < 24 && value_casted != active_config.minute)
  {
    active_config.minute = value_casted;
    time_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "ntpUse"));
  if((value_casted == 1) != active_config.use_ntp)
  {
    ESP_LOGI(TAG3, "casted ntp value %d", value_casted);
    active_config.use_ntp = (value_casted==1);
    time_changed = true;
  }
  else
  {
    ESP_LOGI(TAG3, "unchanged ntp value %s", cJSON_GetObjectItemCaseSensitive(json, "ntpUse")->valuestring);
  }

  if(strcmp(cJSON_GetObjectItemCaseSensitive(json, "timeOffset")->valuestring, "") != 0)
  {
    int16_t value_casted = atoi(cJSON_GetObjectItemCaseSensitive(json, "timeOffset")->valuestring);
    if(value_casted>-5 && value_casted<5 && value_casted != active_config.time_offset)
    {
      ESP_LOGI(TAG3, "casted timeOffset value %d", value_casted);
      active_config.time_offset = value_casted;
      time_changed = true;
    }
    else
    {
      ESP_LOGI(TAG3, "unchanged time_offset value %s", cJSON_GetObjectItemCaseSensitive(json, "timeOffset")->valuestring);
    }
  }
  else
  {
    ESP_LOGI(TAG3, "unchanged time_offset value %s", cJSON_GetObjectItemCaseSensitive(json, "timeOffset")->valuestring);
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c1"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_its_oclock)
  {
    active_config.color_its_oclock = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c2"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_minutes)
  {
    active_config.color_minutes = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c3"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_past_to)
  {
    active_config.color_past_to = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "c4"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_hours)
  {
    active_config.color_hours = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "bri"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.brightness)
  {
    active_config.brightness = value_casted;
    color_changed = true;
  }

  value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "sat"));
  if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.saturation)
  {
    active_config.saturation = value_casted;
    color_changed = true;
  }


  active_config.time_changed = time_changed;
  active_config.color_changed = color_changed;
  active_config.config_changed = color_changed || time_changed;
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

  // Open

  if(active_config.config_changed)
  {
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    esp_err_t err;
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

     // Write
     if(time_changed)
     {
        printf("Updating time values in NVS ... ");
        err = nvs_set_u16(my_handle, "hour", active_config.hour);
        err = nvs_set_u16(my_handle, "minute", active_config.minute);
        uint8_t int_use_ntp = (active_config.use_ntp)?1:0;
        err = nvs_set_u8(my_handle, "int_use_ntp", int_use_ntp);
        err = nvs_set_i16(my_handle, "time_offset", active_config.time_offset);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
     }
     if(color_changed)
     {
        err = nvs_set_u16(my_handle, "color_its_oclock", active_config.color_its_oclock);
        err = nvs_set_u16(my_handle, "color_minutes", active_config.color_minutes);
        err = nvs_set_u16(my_handle, "color_past_to", active_config.color_past_to);
        err = nvs_set_u16(my_handle, "color_hours", active_config.color_hours);
        err = nvs_set_u16(my_handle, "saturation", active_config.saturation);
        err = nvs_set_u16(my_handle, "brightness", active_config.brightness);
     }
    }
  }
}