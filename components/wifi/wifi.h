#ifndef _WIFI_HEADER_
#define _WIFI_HEADER_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_event_base.h>

// ------------------------------------------------------------ Event group -------------------------------------------------------

// event group for wifi events/flags
extern EventGroupHandle_t network_event_group;

// enum for the event bits on the network_event_group group
typedef enum{
	network_eg_wifi_connected 	= 0x01,
	network_eg_mqtt_connected 	= 0x02,
	network_eg_all 				= 0x03,
}network_eg_t;

// ------------------------------------------------------------ Types -------------------------------------------------------------

// struct for wifi connecttion info
typedef struct{
	bool connected;
	char ssid[33];
	int8_t rssi;
	char hostname[33];
	char mac[18];
	char ip[16];
}wifi_info_t;

// return value from wiif_connect_to() call
typedef enum{
	wifi_connect_status_connected = 0,
	wifi_connect_status_timeout,
	wifi_connect_status_error
}wifi_connect_status_t;

// ------------------------------------------------------------ Functions ----------------------------------------------------------

// initialize wifi, net interface, configs and events
void wifi_init();

/**
 * @brief try to connect to wifi ap in wpa2 psk mode
 * @note if already connected to wifi will try and disconnect to connect to new one
 * @param ssid: the network ap name, 32 chars max
 * @param pass: network password, 64 chars max
 * @param timeoutMs: time out in milliseconds
 * @return status, either connected or timeout
 */
wifi_connect_status_t wifi_connect_to(char *ssid, char *pass, size_t timeoutMs);

// check if already connected to a network
bool wifi_connected();

// disconnect from cuurrently connected network
bool wifi_disconnect();

// get wifi info
wifi_info_t wifi_get_info();

// print wifi info
void wifi_print_info();

// get ip/wifi event string
char *wifi_event_to_str(esp_event_base_t event_base, int32_t event_id);

#endif