#include "cmds.h"
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_system.h>
#include <esp_console.h>
#include <argtable3/argtable3.h>
#include <esp_system.h>
#include <esp_app_desc.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <mbedtls/sha256.h>
#include "hexstring.h"
#include <esp_wifi.h>
#include "wifi.h"

// ------------------------------------------------------------ Wifi connect cmd ------------------------------------------------

// argtable struct
static struct{
	struct arg_str *ssid;
	struct arg_str *pass;
	struct arg_int *timeout;
	struct arg_end *end;
}cmd_wifi_join_args;

// wifi_join callback
int cmd_wifi_join(int argq, char **argv){
	int nerrors = arg_parse(argq, argv, (void**) &cmd_wifi_join_args);
	if (nerrors != 0) {
        arg_print_errors(stderr, cmd_wifi_join_args.end, argv[0]);
        return 0;
    }

	if(cmd_wifi_join_args.timeout->count == 0){										// default value for timeout
		cmd_wifi_join_args.timeout->ival[0] = 5000;
	}

	bool res = wifi_connect_to(														// try and connect
		(char *)cmd_wifi_join_args.ssid->sval[0],
		(char *)cmd_wifi_join_args.pass->sval[0],
		(size_t)cmd_wifi_join_args.timeout->ival[0]
	);

	wifi_ap_record_t ap;
	esp_wifi_sta_get_ap_info(&ap);

	if(res){																		// on success
		esp_netif_t *netif = esp_netif_create_default_wifi_sta();
		esp_netif_ip_info_t info;
		esp_netif_get_ip_info(netif, &info);

		printf(
			"Connected to wifi ssid: '%s'\n"
			"Signal strength: %d db"
			"Assinged IP addr: '" IPSTR "'\n", 
			
			ap.ssid,
			ap.rssi,
			IP2STR(&(info.ip))
		);
	}
	else{																			// on fail
		printf("Could not connect to wifi network ssid: '%s'", ap.ssid);
	}

	return 0;
}

// esp command struct 
static const esp_console_cmd_t cmd_wifi_join_cmd = {
    .command = "wifiConnect",
    .help = "Join a wifi network",
    .hint = NULL,
    .func = &cmd_wifi_join,
	.argtable = &cmd_wifi_join_args
};

// ------------------------------------------------------------ Restart cmd --------------------------------------------------------

// restart callback func
int cmd_restart(int argq, char **argv){
    ESP_LOGI(__FILE__, "Restarting...");
    esp_restart();
	return 0;
}

// esp command struct 
static const esp_console_cmd_t cmd_restart_cmd = {
    .command = "restart",
    .help = "Software restart",
    .hint = NULL,
    .func = &cmd_restart,
};

// ------------------------------------------------------------ Login cmd ----------------------------------------------------------

// argtable struct
static struct{
	struct arg_str *pass;
	struct arg_end *end;
}cmd_login_args;

// get info about the firmware and station
void console_print_info(void){
    // get info
    const esp_app_desc_t *info = esp_app_get_description();

    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("credentials", NVS_READONLY, &nvs));

    // get serial
    char serial[37] = {0};
    size_t serialLen; 
    ESP_ERROR_CHECK(nvs_get_str(nvs, "serial", serial, &serialLen));

    nvs_close(nvs);

	char *sha256 = bin2hex((uint8_t*)info->app_elf_sha256, 32, NULL, false);

    printf("\n"
        "Serial: %s\n"
        "Firmware name: %s.\n"
        "Firmware version: %s.\n"
        "Firmware datetime: %s - %s.\n"
        "ESP-IDF version: %s\n"
        "App SHA256: %s\n",

        serial,
        info->project_name,
        info->version,
        info->date, info->time,
        info->idf_ver,
        sha256
    );

	free(sha256);
}

// store credentials read from memory
typedef struct{
	char *serial;
	char *pass;
}credentials_t;

// dealloc credentials struct
void credentials_free(credentials_t *cred){
	free(cred->pass);
	free(cred->serial);
	free(cred);
}

// get password from nvs
credentials_t *nvs_get_credentials(void){
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("credentials", NVS_READONLY, &nvs));

	credentials_t *cred = calloc(sizeof(credentials_t), 1);

	// get serial
	char *serial = calloc(sizeof(char), 37);
	size_t serialLen = 37;
	ESP_ERROR_CHECK(nvs_get_str(nvs, "serial", serial, &serialLen));
	cred->serial = serial;

	// get pass
	char *pass = calloc(sizeof(char), 65);
	size_t passLen = 65;
	ESP_ERROR_CHECK(nvs_get_str(nvs, "pass", pass, &passLen));
	cred->pass = pass;

    nvs_close(nvs);

	return cred;
}

// login callback
int cmd_login(int argq, char **argv){
	int nerrors = arg_parse(argq, argv, (void**) &cmd_login_args);
	if (nerrors != 0) {
        arg_print_errors(stderr, cmd_login_args.end, argv[0]);
        return 0;
    }

	credentials_t *cred = nvs_get_credentials();									// get credential

	const char *input = cmd_login_args.pass->sval[0];
	unsigned char hashBin[33] = {0};
	char *hash;

	char inputSalted[64+36+1] = {0};												// input + salt (serial)
	strncat(inputSalted, input, 64+36);
	strncat(inputSalted, cred->serial, 64+36);
	mbedtls_sha256((const unsigned char *)inputSalted, strlen(inputSalted), hashBin, 0);
	hash = bin2hex(hashBin, 32, NULL, false);										// to hexstring

	if(!strcmp(cred->pass, hash)){													// compare hashes
		printf("You are logged in!\n");
		console_print_info();
	}
	else{
		printf("Given password: '%s' is invalid\n", input);
	}

	credentials_free(cred);
	free(hash);

	return 0;
}

// esp command struct 
static const esp_console_cmd_t cmd_login_cmd = {
    .command = "login",
    .help = "Login with password to unlock other commands",
    .hint = NULL,
    .func = &cmd_login,
	.argtable = &cmd_login_args
};

// ------------------------------------------------------------ Register all commands ----------------------------------------------

// register commands
void cmds_register(void){
	// wifi connect
	cmd_wifi_join_args.ssid    	= arg_str1(NULL, NULL, "<ssid>",    NULL);
	cmd_wifi_join_args.pass    	= arg_str0(NULL, NULL, "<pass>",    NULL);
	cmd_wifi_join_args.timeout 	= arg_int0(NULL, NULL, "<timeout_ms>", NULL);
	cmd_wifi_join_args.end 		= arg_end(10);
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_wifi_join_cmd));
	
	// restart
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_restart_cmd));

	// login
	cmd_login_args.pass 		= arg_str1(NULL, NULL, "<password>", NULL);
	cmd_login_args.end 			= arg_end(10);
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_login_cmd));
}