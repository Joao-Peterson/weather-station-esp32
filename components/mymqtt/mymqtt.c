#include "mymqtt.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "wifi.h"

// global mqtt client instace
esp_mqtt_client_handle_t mqtt_global_client;

// certificate
extern const uint8_t mqtt_server_cert_start[] 	asm("_binary_mosquitto_arch_server_crt_start"); 
extern const uint8_t mqtt_server_cert_end[] 		asm("_binary_mosquitto_arch_server_crt_end"); 

// event handler
static void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

	esp_mqtt_event_handle_t event = event_data;

	switch((esp_mqtt_event_id_t) event_id){
		case MQTT_EVENT_CONNECTED:
			xEventGroupSetBits(network_event_group, network_eg_mqtt_connected);
		break;

		case MQTT_EVENT_DISCONNECTED:
			xEventGroupClearBits(network_event_group, network_eg_mqtt_connected);
		break;

		case MQTT_EVENT_ERROR:
			if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
				ESP_LOGW(__FILE__, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
				ESP_LOGW(__FILE__, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
				ESP_LOGW(__FILE__, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
						strerror(event->error_handle->esp_transport_sock_errno));
			} else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
				ESP_LOGW(__FILE__, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
			} else {
				ESP_LOGW(__FILE__, "Unknown error type: 0x%x", event->error_handle->error_type);
			}
		break;

		default:
		break;
	}
}

// init
void mqtt_init(const char *uri, const char *user, const char *pass){
	esp_mqtt_client_config_t cfg = {
		.credentials = {
			.username = user,
			.authentication = {
				.password = pass
			}	
		},
		.network = {
			.disable_auto_reconnect = false,
			.reconnect_timeout_ms = 5000
		},
		.broker = {
			.address = {
				.uri = uri,
			},
			.verification = {
				.certificate = (const char*)mqtt_server_cert_start,
				.skip_cert_common_name_check = true
			}
		}
	};

	mqtt_global_client = esp_mqtt_client_init(&cfg);

	esp_mqtt_client_register_event(mqtt_global_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

	esp_mqtt_client_start(mqtt_global_client);
}

// send float
void mqtt_send_float(float value, const char *topic){
	char valueStr[15];

	snprintf(valueStr, 14, "% #.5g", value);

	esp_mqtt_client_publish(mqtt_global_client, topic, (const char*)valueStr, 0, 0, 0);
}