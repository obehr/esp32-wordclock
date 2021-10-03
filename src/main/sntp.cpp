/* LwIP SNTP example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

static const char *TAG4 = "sntp";


void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG4, "Notification of a time synchronization event");
    if(sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
    {
        ESP_LOGI(TAG4, "Time was synched (again)");
        time_t now;
        struct tm timeinfo;
        char strftime_buf[64];

        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG4, "The time after verified sync: %s", strftime_buf);
    }
    else
    {
        ESP_LOGI(TAG4, "Time sync process is ongoing or was reset");
    }
}

static void initialize_sntp()
{
    ESP_LOGI(TAG4, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "fritz.box");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    sntp_init();
}

static bool wait_for_sync()
{
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && ++retry < retry_count) {
        ESP_LOGI(TAG4, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if(sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
    {
        ESP_LOGI(TAG4, "Time was retrieved");
        return true;
    }
    else
    {
        ESP_LOGI(TAG4, "Time sync process is ongoing or was reset");
        return false;
    }
    
}