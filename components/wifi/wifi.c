#include "wifi.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// ------------------------------------------------------------ Globals ------------------------------------------------------------

EventGroupHandle_t wifi_event_group;

// ------------------------------------------------------------ Private functions --------------------------------------------------

// handler for the wifi and ip events
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
	if(event_base == WIFI_EVENT){													// wifi events
		switch(event_id){
			case WIFI_EVENT_STA_START:												// after wifi start
			case WIFI_EVENT_STA_DISCONNECTED:										// if wifi disconnected
				esp_wifi_connect();													// try to reconnect
				xEventGroupClearBits(wifi_event_group, wifi_eg_connected);
			break;
		}
	}
	else if(event_base == IP_EVENT){												// ip events
		switch(event_id){
			case IP_EVENT_STA_GOT_IP:												// on got ipv4
				xEventGroupSetBits(wifi_event_group, wifi_eg_connected);
			break;
		}
	}
}

// ------------------------------------------------------------ Fuctions -----------------------------------------------------------

void wifi_init(){
    wifi_event_group = xEventGroupCreate();											// create event group
	xEventGroupClearBits(wifi_event_group, wifi_eg_connected);

    ESP_ERROR_CHECK(esp_netif_init());												// initialize net interface
    ESP_ERROR_CHECK(esp_event_loop_create_default());								// default esp event loop

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();							// default wifi init config
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, 							// register handler for wifi events
		ESP_EVENT_ANY_ID, 
		wifi_event_handler, NULL)
	);

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, 							// register handler for ip events
		ESP_EVENT_ANY_ID, 
		wifi_event_handler, NULL)
	);

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));						// auto record wifi ssid and pass
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));								// station/client mode
    ESP_ERROR_CHECK(esp_wifi_start());												// start wifi interface
}

bool wifi_connected(){
	int wifi_group = xEventGroupGetBits(wifi_event_group);							// get wifi group
	return !!(wifi_group & wifi_eg_connected);										// return the connected bit
}

bool wifi_disconnect(){
	return esp_wifi_disconnect() == ESP_OK ? true : false;							// try to disconnect
}

bool wifi_connect_to(char *ssid, char *pass, size_t timeoutMs){
	if(ssid == NULL || pass == NULL) return false;

	if(wifi_connected()){
		wifi_disconnect();															// try to disconnect if already connected
	}
	
	wifi_config_t config = {														// wifi config
		.sta = {
			.threshold.authmode = WIFI_AUTH_WPA2_PSK								// wpa2 psk
		}
	};

	strlcpy((char*)config.sta.ssid, ssid, sizeof(config.sta.ssid));						// copy ssid

	if(pass != NULL)
		strlcpy((char*)config.sta.password, pass, sizeof(config.sta.password));			// copy pass if passed

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));						// set conf
	esp_wifi_connect();																// try to connect

	int wifi_group = xEventGroupWaitBits(											// wait for connection
		wifi_event_group, 
		wifi_eg_connected,
		pdFALSE, pdTRUE,
		timeoutMs / portTICK_PERIOD_MS
	);

	return !!(wifi_group & wifi_eg_connected);										// return the actual bit
}