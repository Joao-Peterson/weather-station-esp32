#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "console_task.h"
#include "wifi.h"

void app_main(void){
    // initialize flash
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // initialize wifi
    wifi_init();

    // run console task
    xTaskCreate(console_task, "console_task", 4096, NULL, 0, NULL);
}
