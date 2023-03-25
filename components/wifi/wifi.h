#ifndef _WIFI_HEADER_
#define _WIFI_HEADER_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// ------------------------------------------------------------ Event group -------------------------------------------------------

// event group for wifi events/flags
extern EventGroupHandle_t wifi_event_group;

// enum for the event bits on the wifi_event_group group
typedef enum{
	wifi_eg_connected =			0x01,
}wifi_eg_t;

// ------------------------------------------------------------ Functions ----------------------------------------------------------

// initialize wifi, net interface, configs and events
void wifi_init();

/**
 * @brief try to connect to wifi ap in wpa2 psk mode
 * @note if already connected to wifi will try and disconnect to connect to new one
 * @param ssid: the network ap name, 32 chars max
 * @param pass: network password, 64 chars max
 * @param timeoutMs: time out in milliseconds
 * @return true if successful or false otherwise
 */
bool wifi_connect_to(char *ssid, char *pass, size_t timeoutMs);

// check if already connected to a network
bool wifi_connected();

// disconnect from cuurrently connected network
bool wifi_disconnect();

#endif