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
  bool use_ntp;
  uint16_t year;
  uint16_t month;
  uint16_t day;
  uint16_t hour;
  uint16_t minute;
  int16_t time_offset;
  uint16_t color_its_oclock;
  uint16_t color_minutes;
  uint16_t color_past_to;
  uint16_t color_hours;
  uint16_t saturation;
  uint16_t brightness;
  bool time_changed;
  bool color_changed;
  bool brightness_changed;
  bool saturation_changed;
  bool config_changed;
  bool set_time;
  bool set_colors;
  bool set_brightness;
  bool set_saturation;
} my_config;


static bool config_available = false;

static my_config valid_config;

static my_config default_config;

static my_config active_config;

static int32_t restart_counter = 0;

static void init_config()
{
  ESP_LOGI(TAG3, "Read config from nvs");
  active_config.set_time = false;
  active_config.set_colors = false;
  active_config.set_brightness = false;
  active_config.set_saturation = false;
  active_config.year = 0;
  active_config.month = 0;
  active_config.day = 0;
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
  err = nvs_open("storage", NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGI(TAG3, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    // Read
    
    err = nvs_get_u16(my_handle, "year", &active_config.year);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read year from NVS %d", active_config.year); }

    err = nvs_get_u16(my_handle, "month", &active_config.month);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read month from NVS %d", active_config.month); }

    err = nvs_get_u16(my_handle, "day", &active_config.day);
    if(err == ESP_OK)
    { ESP_LOGI(TAG3, "Read day from NVS %d", active_config.day); }

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

static bool get_stripe_status()
{
  ESP_LOGI(TAG3, "Get stripe status");
  bool status = false;
  uint8_t int_status;
  esp_err_t err;
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGI(TAG3, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    err = nvs_get_u8(my_handle, "status_stripe", &int_status);
    if(err == ESP_OK)
    { 
      status = (int_status==1);
      ESP_LOGI(TAG3, "Read status_stripe from NVS %d", status);
    }
  }
  return status;
}

static bool get_display_status()
{
  ESP_LOGI(TAG3, "Get display status");
  bool status = false;
  uint8_t int_status;
  esp_err_t err;
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGI(TAG3, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    err = nvs_get_u8(my_handle, "status_display", &int_status);
    if(err == ESP_OK)
    { 
      status = (int_status==1);
      ESP_LOGI(TAG3, "Read status_display from NVS %d", status);
    }
  }
  return status;
}

static void set_stripe_status(bool status)
{
  ESP_LOGI(TAG3, "Set stripe status %d", status);
  uint8_t int_status = (status)?1:0;
  esp_err_t err;
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
      printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
     err = nvs_set_u8(my_handle, "status_stripe", int_status);
  }
}

static void set_display_status(bool status)
{
  ESP_LOGI(TAG3, "Set display status %d", status);
  uint8_t int_status = (status)?1:0;
  esp_err_t err;
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
      printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
     err = nvs_set_u8(my_handle, "status_display", int_status);
  }
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
  if(cJSON_IsString(json_object) && strcmp(json_object->valuestring, "") != 0)
  {
    int16_t casted_object = atoi(json_object->valuestring);
    if(casted_object >= 0)
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
    "section":"0",
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
  cJSON *json_object;
  char *string = cJSON_Print(json);
  ESP_LOGI(TAG3, "parsed json %s", string);
  //cJSON *root = cJSON_Parse(content);
  //content[recv_size] = '\0';

  bool time_changed = false;
  bool color_changed = false;
  bool brightness_changed = false;
  bool saturation_changed = false;
  int16_t value_casted;
  
  bool set_time = cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json, "set_time"));
  ESP_LOGI(TAG3, "bool set_time %d", set_time);
  bool set_colors = cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json, "set_colors"));
  ESP_LOGI(TAG3, "bool set_brightness %d", set_colors);
  bool set_brightness = cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json, "set_brightness"));
  ESP_LOGI(TAG3, "bool set_brightness %d", set_brightness);
  bool set_saturation = cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json, "set_saturation"));
  ESP_LOGI(TAG3, "bool set_saturation %d", set_saturation);

  if(set_time)
  {
    json_object = cJSON_GetObjectItemCaseSensitive(json, "use_ntp");
    if(cJSON_IsString(json_object))
    {
      value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "time_offset"));
      if(value_casted != -1 && value_casted < 7 && value_casted != active_config.time_offset)
      {
        ESP_LOGI(TAG3, "casted time_offset value %d", value_casted);
        active_config.time_offset = value_casted;
        time_changed = true;
      }

      bool use_ntp = strcmp(json_object->valuestring, "1") == 0;
      ESP_LOGI(TAG3, "casted use_ntp value %d", use_ntp);
      if(active_config.use_ntp != use_ntp)
      {
        ESP_LOGI(TAG3, "use_ntp value changed");
        time_changed = true;
        active_config.use_ntp = use_ntp;  
      }
    }

    if(active_config.use_ntp) //network time setup
    { 
      ESP_LOGI(TAG3, "Process network time config");

      
    }
    else //manual time setup
    {
      ESP_LOGI(TAG3, "Process manual time config");

      value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "year"));
      if(value_casted != -1 && value_casted > 2019 && value_casted < 2038 && value_casted != active_config.year)
      {
        ESP_LOGI(TAG3, "casted year value %d", value_casted);
        active_config.year = value_casted;
        time_changed = true;
      }

      value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "month"));
      if(value_casted > 0 && value_casted < 13 && value_casted != active_config.month)
      {
        ESP_LOGI(TAG3, "casted month value %d", value_casted);
        active_config.month = value_casted;
        time_changed = true;
      }

      value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "day"));
      if(value_casted > 0 && value_casted < 32 && value_casted != active_config.day)
      {
        ESP_LOGI(TAG3, "casted day value %d", value_casted);
        active_config.day = value_casted;
        time_changed = true;
      }

      value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "hour"));
      if(value_casted != -1 && value_casted < 24 && value_casted != active_config.hour)
      {
        ESP_LOGI(TAG3, "casted hour value %d", value_casted);
        active_config.hour = value_casted;
        time_changed = true;
      }

      value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "minute"));
      if(value_casted != -1 && value_casted < 60 && value_casted != active_config.minute)
      {
        ESP_LOGI(TAG3, "casted minute value %d", value_casted);
        active_config.minute = value_casted;
        time_changed = true;
      }
    }
  }

  if(set_colors) //color setup
  {
    ESP_LOGI(TAG3, "Process color config");
  
    value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "color_its_oclock"));
    if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_its_oclock)
    {
      active_config.color_its_oclock = value_casted;
      color_changed = true;
    }

    value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "color_minutes"));
    if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_minutes)
    {
      active_config.color_minutes = value_casted;
      color_changed = true;
    }

    value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "color_past_to"));
    if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_past_to)
    {
      active_config.color_past_to = value_casted;
      color_changed = true;
    }

    value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "color_hours"));
    if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.color_hours)
    {
      active_config.color_hours = value_casted;
      color_changed = true;
    }
  }

  if(set_brightness) //brightness setup
  {
    ESP_LOGI(TAG3, "Process color config");

    value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "bri"));
    if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.brightness)
    {
      active_config.brightness = value_casted;
      brightness_changed = true;
    }
  }

  if(set_saturation) //saturation setup
  {
    ESP_LOGI(TAG3, "Process color config");
    value_casted = get_number(cJSON_GetObjectItemCaseSensitive(json, "sat"));
    if(value_casted != -1 && value_casted <= 255 && value_casted != active_config.saturation)
    {
      active_config.saturation = value_casted;
      saturation_changed = true;
    }
  }

  active_config.time_changed = time_changed;
  active_config.color_changed = color_changed;
  active_config.brightness_changed = brightness_changed;
  active_config.saturation_changed = saturation_changed;
  active_config.config_changed = color_changed || time_changed || brightness_changed || saturation_changed;
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
        err = nvs_set_u8(my_handle, "use_ntp", active_config.use_ntp);
        if(active_config.use_ntp)
        {
          err = nvs_set_i16(my_handle, "time_offset", active_config.time_offset);
        }
        else
        {
          err = nvs_set_u16(my_handle, "hour", active_config.hour);
          err = nvs_set_u16(my_handle, "minute", active_config.minute);
          err = nvs_set_u16(my_handle, "year", active_config.year);
          err = nvs_set_u16(my_handle, "month", active_config.month);
          err = nvs_set_u16(my_handle, "day", active_config.day);
        }
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
     }
     if(color_changed)
     {
        err = nvs_set_u16(my_handle, "color_its_oclock", active_config.color_its_oclock);
        err = nvs_set_u16(my_handle, "color_minutes", active_config.color_minutes);
        err = nvs_set_u16(my_handle, "color_past_to", active_config.color_past_to);
        err = nvs_set_u16(my_handle, "color_hours", active_config.color_hours);
     }
     if(brightness_changed)
     {
        err = nvs_set_u16(my_handle, "brightness", active_config.brightness);
     }
     if(saturation_changed)
     {
        err = nvs_set_u16(my_handle, "saturation", active_config.saturation);  
     }
    }
  }
}