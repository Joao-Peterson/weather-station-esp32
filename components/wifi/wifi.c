#include "wifi.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdkconfig.h"
#include "cred.h"

// ------------------------------------------------------------ Defines ------------------------------------------------------------

// mac utils
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC2STR(mac) mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]

// error utils
#define NEW_ENUM_STR(x) {.value = (int)x, .string = #x}
#define NEW_ENUM_STR_TERMINATOR {0}

typedef struct{
	int value;
	char *string;
}enum_str_t;

// ------------------------------------------------------------ Globals ------------------------------------------------------------

// global wifi event group
EventGroupHandle_t wifi_event_group;

// global wifi sta interface
esp_netif_t *netif_sta = NULL; 

// wifi err enum_string
const enum_str_t wifi_err_reason_str[] = {
    NEW_ENUM_STR(WIFI_REASON_UNSPECIFIED),
    NEW_ENUM_STR(WIFI_REASON_AUTH_EXPIRE),
    NEW_ENUM_STR(WIFI_REASON_AUTH_LEAVE),
    NEW_ENUM_STR(WIFI_REASON_ASSOC_EXPIRE),
    NEW_ENUM_STR(WIFI_REASON_ASSOC_TOOMANY),
    NEW_ENUM_STR(WIFI_REASON_NOT_AUTHED),
    NEW_ENUM_STR(WIFI_REASON_NOT_ASSOCED),
    NEW_ENUM_STR(WIFI_REASON_ASSOC_LEAVE),
    NEW_ENUM_STR(WIFI_REASON_ASSOC_NOT_AUTHED),
    NEW_ENUM_STR(WIFI_REASON_DISASSOC_PWRCAP_BAD),
    NEW_ENUM_STR(WIFI_REASON_DISASSOC_SUPCHAN_BAD),
    NEW_ENUM_STR(WIFI_REASON_BSS_TRANSITION_DISASSOC),
    NEW_ENUM_STR(WIFI_REASON_IE_INVALID),
    NEW_ENUM_STR(WIFI_REASON_MIC_FAILURE),
    NEW_ENUM_STR(WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT),
    NEW_ENUM_STR(WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT),
    NEW_ENUM_STR(WIFI_REASON_IE_IN_4WAY_DIFFERS),
    NEW_ENUM_STR(WIFI_REASON_GROUP_CIPHER_INVALID),
    NEW_ENUM_STR(WIFI_REASON_PAIRWISE_CIPHER_INVALID),
    NEW_ENUM_STR(WIFI_REASON_AKMP_INVALID),
    NEW_ENUM_STR(WIFI_REASON_UNSUPP_RSN_IE_VERSION),
    NEW_ENUM_STR(WIFI_REASON_INVALID_RSN_IE_CAP),
    NEW_ENUM_STR(WIFI_REASON_802_1X_AUTH_FAILED),
    NEW_ENUM_STR(WIFI_REASON_CIPHER_SUITE_REJECTED),
    NEW_ENUM_STR(WIFI_REASON_TDLS_PEER_UNREACHABLE),
    NEW_ENUM_STR(WIFI_REASON_TDLS_UNSPECIFIED),
    NEW_ENUM_STR(WIFI_REASON_SSP_REQUESTED_DISASSOC),
    NEW_ENUM_STR(WIFI_REASON_NO_SSP_ROAMING_AGREEMENT),
    NEW_ENUM_STR(WIFI_REASON_BAD_CIPHER_OR_AKM),
    NEW_ENUM_STR(WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION),
    NEW_ENUM_STR(WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS),
    NEW_ENUM_STR(WIFI_REASON_UNSPECIFIED_QOS),
    NEW_ENUM_STR(WIFI_REASON_NOT_ENOUGH_BANDWIDTH),
    NEW_ENUM_STR(WIFI_REASON_MISSING_ACKS),
    NEW_ENUM_STR(WIFI_REASON_EXCEEDED_TXOP),
    NEW_ENUM_STR(WIFI_REASON_STA_LEAVING),
    NEW_ENUM_STR(WIFI_REASON_END_BA),
    NEW_ENUM_STR(WIFI_REASON_UNKNOWN_BA),
    NEW_ENUM_STR(WIFI_REASON_TIMEOUT),
    NEW_ENUM_STR(WIFI_REASON_PEER_INITIATED),
    NEW_ENUM_STR(WIFI_REASON_AP_INITIATED),
    NEW_ENUM_STR(WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT),
    NEW_ENUM_STR(WIFI_REASON_INVALID_PMKID),
    NEW_ENUM_STR(WIFI_REASON_INVALID_MDE),
    NEW_ENUM_STR(WIFI_REASON_INVALID_FTE),
    NEW_ENUM_STR(WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED),
    NEW_ENUM_STR(WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED),
    NEW_ENUM_STR(WIFI_REASON_BEACON_TIMEOUT),
    NEW_ENUM_STR(WIFI_REASON_NO_AP_FOUND),
    NEW_ENUM_STR(WIFI_REASON_AUTH_FAIL),
    NEW_ENUM_STR(WIFI_REASON_ASSOC_FAIL),
    NEW_ENUM_STR(WIFI_REASON_HANDSHAKE_TIMEOUT),
    NEW_ENUM_STR(WIFI_REASON_CONNECTION_FAIL),
    NEW_ENUM_STR(WIFI_REASON_AP_TSF_RESET),
    NEW_ENUM_STR(WIFI_REASON_ROAMING),
    NEW_ENUM_STR(WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG),
	NEW_ENUM_STR_TERMINATOR
};

// wifi event enum_strin
const enum_str_t wifi_event_str[] = {
    NEW_ENUM_STR(WIFI_EVENT_WIFI_READY),
    NEW_ENUM_STR(WIFI_EVENT_SCAN_DONE),
    NEW_ENUM_STR(WIFI_EVENT_STA_START),
    NEW_ENUM_STR(WIFI_EVENT_STA_STOP),
    NEW_ENUM_STR(WIFI_EVENT_STA_CONNECTED),
    NEW_ENUM_STR(WIFI_EVENT_STA_DISCONNECTED),
    NEW_ENUM_STR(WIFI_EVENT_STA_AUTHMODE_CHANGE),
	NEW_ENUM_STR(WIFI_EVENT_STA_WPS_ER_SUCCESS),
    NEW_ENUM_STR(WIFI_EVENT_STA_WPS_ER_FAILED),
    NEW_ENUM_STR(WIFI_EVENT_STA_WPS_ER_TIMEOUT),
    NEW_ENUM_STR(WIFI_EVENT_STA_WPS_ER_PIN),
    NEW_ENUM_STR(WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP),
	NEW_ENUM_STR(WIFI_EVENT_AP_START),
    NEW_ENUM_STR(WIFI_EVENT_AP_STOP),
    NEW_ENUM_STR(WIFI_EVENT_AP_STACONNECTED),
    NEW_ENUM_STR(WIFI_EVENT_AP_STADISCONNECTED),
    NEW_ENUM_STR(WIFI_EVENT_AP_PROBEREQRECVED),
	NEW_ENUM_STR(WIFI_EVENT_FTM_REPORT),
    NEW_ENUM_STR(WIFI_EVENT_STA_BSS_RSSI_LOW),
    NEW_ENUM_STR(WIFI_EVENT_ACTION_TX_STATUS),
    NEW_ENUM_STR(WIFI_EVENT_ROC_DONE),
	NEW_ENUM_STR(WIFI_EVENT_STA_BEACON_TIMEOUT),
	NEW_ENUM_STR(WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START),
	NEW_ENUM_STR(WIFI_EVENT_AP_WPS_RG_SUCCESS),
    NEW_ENUM_STR(WIFI_EVENT_AP_WPS_RG_FAILED),
    NEW_ENUM_STR(WIFI_EVENT_AP_WPS_RG_TIMEOUT),
    NEW_ENUM_STR(WIFI_EVENT_AP_WPS_RG_PIN),
    NEW_ENUM_STR(WIFI_EVENT_AP_WPS_RG_PBC_OVERLAP),
	NEW_ENUM_STR_TERMINATOR
};

// ip event enum_strin
const enum_str_t ip_event_str[] = {
    NEW_ENUM_STR(IP_EVENT_STA_GOT_IP),
    NEW_ENUM_STR(IP_EVENT_STA_LOST_IP),
    NEW_ENUM_STR(IP_EVENT_AP_STAIPASSIGNED),
    NEW_ENUM_STR(IP_EVENT_GOT_IP6),
    NEW_ENUM_STR(IP_EVENT_ETH_GOT_IP),
    NEW_ENUM_STR(IP_EVENT_ETH_LOST_IP),
    NEW_ENUM_STR(IP_EVENT_PPP_GOT_IP),
    NEW_ENUM_STR(IP_EVENT_PPP_LOST_IP),
	NEW_ENUM_STR_TERMINATOR
};

// signal strength
const char *rssi_strength_str[] = {
	"Impossibly weak (WTF\?\?)",
	"Very weak signal (risk of disconnection)",
	"Weak signal",
	"Good signal",
	"Strong signal",
	"Very strong signal",
	"Super strong signal (how did you manage this\?\?\?)",
	"No signal (no connection)"
};

// ------------------------------------------------------------ Private functions --------------------------------------------------

// get string from array of enum_str
char *enum_str_get_string(const enum_str_t *array, int number){
	int i = 0;
	while(array[i].string != NULL){
		if(array[i].value == number)
			return array[i].string;

		i++;
	}

	return NULL;
}

// handler for the wifi and ip events
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
	
	ESP_LOGD("wifi", "[DEBUG] [%s.%i] (%s) wifi_event: %s \n", __FILE__, __LINE__, __CC_SUPPORTS___FUNC__ ? __func__ : "some_function", wifi_event_to_str(event_base, event_id));
	
	if(event_base == WIFI_EVENT){													// wifi events
	
		switch(event_id){
			case WIFI_EVENT_STA_START:												// after wifi start
			// case WIFI_EVENT_STA_CONNECTED:											// after wifi start
				esp_err_t res = esp_wifi_connect();													// try to reconnect
				ESP_LOGD("wifi", "[DEBUG] [%s.%i] (%s) esp_wifi_connect: %s \n", __FILE__, __LINE__, __CC_SUPPORTS___FUNC__ ? __func__ : "some_function", esp_err_to_name(res));
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

// init
void wifi_init(){
    esp_log_level_set("wifi", ESP_LOG_WARN);										// internal esp wifi logs to warning, too much info

    wifi_event_group = xEventGroupCreate();											// create event group
	xEventGroupClearBits(wifi_event_group, wifi_eg_connected);

    ESP_ERROR_CHECK(esp_netif_init());												// initialize net interface
    ESP_ERROR_CHECK(esp_event_loop_create_default());								// default esp event loop

	netif_sta = esp_netif_create_default_wifi_sta();									// create interface
	assert(netif_sta);
	
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

	credentials_t *cred = credentials_get_nvs();									// get serial

	char hostname[33] = {0};
	snprintf((char*)hostname, 32, CONFIG_HOSTNAME, cred->serial);					// hostname
	esp_netif_set_hostname(netif_sta, hostname);
	credentials_free(cred);
}

// get connected status
bool wifi_connected(){
	int wifi_group = xEventGroupGetBits(wifi_event_group);							// get wifi group
	return !!(wifi_group & wifi_eg_connected);										// return the connected bit
}

// disconnect
bool wifi_disconnect(){
	return esp_wifi_disconnect() == ESP_OK ? true : false;							// try to disconnect
}

// connect to ap
wifi_connect_status_t wifi_connect_to(char *ssid, char *pass, size_t timeoutMs){
	if(ssid == NULL || pass == NULL) return false;

	if(wifi_connected()){
		bool a = wifi_disconnect();															// try to disconnect if already connected
		printf("[%s.%i] (%s) [DEBUG] disconnect: %s \n", __FILE__, __LINE__, __CC_SUPPORTS___FUNC__ ? __func__ : "some_function", a ? "true" : "false");
	}
	
	wifi_config_t config = {														// wifi config
		.sta = {
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,								// wpa2 psk
			.pmf_cfg = {
				.capable = true,
				.required = false
			}
		}
	};

	strlcpy((char*)config.sta.ssid, ssid, sizeof(config.sta.ssid));					// copy ssid

	if(pass != NULL)
		strlcpy((char*)config.sta.password, pass, sizeof(config.sta.password));		// copy pass if passed

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));						// set conf

	esp_err_t res = esp_wifi_connect();												// try to connect

	if(res){																		// on any error
		ESP_LOGE("wifi", "%s\n", esp_err_to_name(res));
		return wifi_connect_status_error;
	}

	int wifi_group = xEventGroupWaitBits(											// wait for connection
		wifi_event_group, 
		wifi_eg_connected,
		pdFALSE, pdTRUE,
		timeoutMs / portTICK_PERIOD_MS
	);

	if(wifi_group & wifi_eg_connected){												// if bit connected
		return wifi_connect_status_connected;
	}
	else{																			// else,r eturned by timeout
		return wifi_connect_status_timeout;
	}
}

// get wifi info
wifi_info_t wifi_get_info(){
	wifi_info_t info = {
		.connected = false,
		.mac = "",
		.hostname = "",
		.ip = "",
		.ssid = "",
		.rssi = -1,
	};

	uint8_t mac[6];
	esp_netif_get_mac(netif_sta, mac);
	snprintf(info.mac, 18, MACSTR, MAC2STR(mac));									// mac
	
	const char *hostname = NULL;
	esp_netif_get_hostname(netif_sta, &hostname);									// hostname
	strncpy(info.hostname, hostname == NULL ? "<hostname NULL>" : hostname, 33);

	wifi_ap_record_t ap;
	esp_err_t apRes = esp_wifi_sta_get_ap_info(&ap);								// get ap info

	if(apRes) return info;															// error on ap info
	
	if(netif_sta == NULL) return info;
	esp_netif_ip_info_t netifInfo;
	if(esp_netif_get_ip_info(netif_sta, &netifInfo)){
		snprintf(info.ip, 16, "0.0.0.0");
	}


	int wifi_group = xEventGroupGetBits(wifi_event_group);							// get event bits
	
	info.connected = (bool)(wifi_group & wifi_eg_connected);						// connected
	strncpy(info.ssid, (char *)ap.ssid, 32);										// ssid
	info.rssi = ap.rssi;															// rssi
	snprintf(info.ip, 16, IPSTR, IP2STR(&(netifInfo.ip)));							// ip

	return info;
}

// print wifi info
void wifi_print_info(){
	wifi_info_t info = wifi_get_info();

	int rssi = -info.rssi;
	const char *rssiStr = rssi_strength_str[0];

	if(rssi == 1)
		rssiStr = rssi_strength_str[7];
	else if(rssi >= 0 && rssi < 40)
		rssiStr = rssi_strength_str[6];
	else if(rssi >= 40 && rssi < 50)
		rssiStr = rssi_strength_str[5];
	else if(rssi >= 50 && rssi < 60)
		rssiStr = rssi_strength_str[4];
	else if(rssi >= 60 && rssi < 70)
		rssiStr = rssi_strength_str[3];
	else if(rssi >= 70 && rssi < 80)
		rssiStr = rssi_strength_str[2];
	else if(rssi >= 80 && rssi < 90)
		rssiStr = rssi_strength_str[1];
	else if(rssi >= 90)
		rssiStr = rssi_strength_str[0];

	printf(
		"MAC: %s\n"
		"Hostname: %s\n"
		"Wifi status: %s\n"
		"SSID: '%s'\n"
		"Signal strength: (%d dbm) - \"%s\"\n"
		"IP: %s\n",

		info.mac,
		info.hostname,
		info.connected ? "Connected" : "Innactive",
		info.ssid,
		info.rssi, rssiStr,
		info.ip
	);
}

// get ip/wifi event string
char *wifi_event_to_str(esp_event_base_t event_base, int32_t event_id){
	if(event_base == WIFI_EVENT){
		return enum_str_get_string(wifi_event_str, (int)event_id);
	}
	
	if(event_base == IP_EVENT){
		return enum_str_get_string(ip_event_str, (int)event_id);
	}

	return NULL;
}