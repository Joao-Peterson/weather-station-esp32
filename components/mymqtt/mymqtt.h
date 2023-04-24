#ifndef _MYMQTT_HEADER_
#define _MYMQTT_HEADER_
#include <mqtt5_client.h>

// ------------------------------------------------------------ Macros -------------------------------------------------------------

extern esp_mqtt_client_handle_t mqtt_global_client;

// ------------------------------------------------------------ Macros -------------------------------------------------------------

// send float
void mqtt_send_float(float value, const char *topic);

// ------------------------------------------------------------ Functions ----------------------------------------------------------

/**
 * @brief init mqtt client
 */
void mqtt_init(const char *uri, const char *user, const char *pass);

#endif