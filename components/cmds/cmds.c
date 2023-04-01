#include "cmds.h"
#include <string.h>
#include <stdbool.h>
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
#include <esp_wifi.h>
#include "hexstring.h"
#include "wifi.h"
#include "cred.h"
#include "strswitch.h"

// ------------------------------------------------------------ Globals ---------------------------------------------------------

bool logged = false;

// ------------------------------------------------------------ Wifi cmd --------------------------------------------------------

// arguments wifi command
static struct{
	struct arg_str *subcommand;
	struct arg_str *ssid;
	struct arg_str *pass;
	struct arg_int *timeout;
	struct arg_end *end;
}cmd_wifi_args;

// wifi disconnect commands callback
int cmd_wifi_disconnect(int argq, char **argv){
	if(wifi_disconnect())
		printf("Disconnected from wifi ap\n");
	else
		printf("Already not connected to any wifi ap\n");

	return 0;
}

// wifi connect commands callback
int cmd_wifi_connect(int argq, char **argv){
	int nerrors = arg_parse(argq, argv, (void**) &cmd_wifi_args);
	if (nerrors != 0) {
        arg_print_errors(stderr, cmd_wifi_args.end, argv[0]);
        return 0;
    }

	if(cmd_wifi_args.ssid->count == 0){											// if no ssid
		printf(
			"When using 'wifi connect' please pass an ssid and optinally an password and timeout\n"
		);

		return 0;
	}

	if(cmd_wifi_args.timeout->count == 0){											// default value for timeout
		cmd_wifi_args.timeout->ival[0] = 5000;
	}

	wifi_connect_status_t res = wifi_connect_to(									// try and connect
		(char *)cmd_wifi_args.ssid->sval[0],
		(char *)cmd_wifi_args.pass->sval[0],
		(size_t)cmd_wifi_args.timeout->ival[0]
	);

	switch(res){
		case wifi_connect_status_connected:											// if connect ok
			wifi_print_info();
		break;

		case wifi_connect_status_timeout:											// timeout
			printf(
				"Could not connect to wifi network ssid: '%s' timeout reached [%d ms]\n", 
				(char *)cmd_wifi_args.ssid->sval[0],
				(int)cmd_wifi_args.timeout->ival[0]
			);
		break;
		
		default:
		case wifi_connect_status_error:												// on error
			printf(
				"Error connecting to wifi network ssid: '%s'\n", 
				(char *)cmd_wifi_args.ssid->sval[0]
			);
		break;
	}

	return 0;
}

// wifi info command callback
int cmd_wifi_info(int argq, char **argv){
	wifi_print_info();
	return 0;
}

// main wifi commands callback
int cmd_wifi(int argq, char **argv){
	if(!logged){
		printf("Please login first before executing this priviledged command\n");
		return 0;
	}
	
	char *subcommand = argv[1];

	if(subcommand == NULL) subcommand = "info";										// default subcommand

	sswitch(subcommand){															// dispatch subcommand
		scase("info")
			return cmd_wifi_info(argq, argv);
		sbreak

		scase("connect")
			return cmd_wifi_connect(argq, argv);
		sbreak

		scase("disconnect")
			return cmd_wifi_disconnect(argq, argv);
		sbreak

		sdefault																	// invalid subcommand
			printf("subcommand '%s' doesn't exist\n", subcommand);
			return 0;
		sbreak
	}
}

// wifi command struct
static const esp_console_cmd_t cmd_wifi_cmd = {
    .command = "wifi",
    .help = "Wifi commands",
    .hint = NULL,
    .func = &cmd_wifi,
	.argtable = &cmd_wifi_args
};

// ------------------------------------------------------------ Restart cmd --------------------------------------------------------

// restart callback func
int cmd_restart(int argq, char **argv){
	if(!logged){
		printf("Please login first before executing this priviledged command\n");
		return 0;
	}

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

	credentials_t *cred = credentials_get_nvs();									// get credentials

	char *sha256 = bin2hex((uint8_t*)info->app_elf_sha256, 32, NULL, false);

    printf(
        "Serial: %s\n"
        "Firmware name: %s.\n"
        "Firmware version: %s.\n"
        "Firmware datetime: %s - %s.\n"
        "ESP-IDF version: %s\n"
        "App SHA256: %s\n",

        cred->serial,
        info->project_name,
        info->version,
        info->date, info->time,
        info->idf_ver,
        sha256
    );

	free(sha256);
	credentials_free(cred);
}

// login callback
int cmd_login(int argq, char **argv){
	if(logged){
		printf("You are already logged in\n");
		return 0;
	}

	int nerrors = arg_parse(argq, argv, (void**) &cmd_login_args);
	if (nerrors != 0) {
        arg_print_errors(stderr, cmd_login_args.end, argv[0]);
        return 0;
    }

	credentials_t *cred = credentials_get_nvs();									// get credentials

	char *input = (char*)cmd_login_args.pass->sval[0];
	bool res = credentials_check_pass(input);

	if(res){																		// password correct
		printf("You are logged in!\n");
		logged = true;
		// console_print_info();
	}
	else{
		printf("Given password: '%s' is invalid\n", input);
	}

	credentials_free(cred);

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

// ------------------------------------------------------------ Exit cmd -----------------------------------------------------------

// exit command callback
int cmd_exit(int argq, char **argv){
	if(!logged){
		printf("You are not logged in, no need to exit\n");
	}
	else{
		printf("You exited the session\n");
		logged = false;
	}

	return 0;
}

// exit command struct 
static const esp_console_cmd_t cmd_exit_cmd = {
    .command = "exit",
    .help = "Exit, sign out of login screen",
    .hint = NULL,
    .func = &cmd_exit,
	.argtable = NULL
};

// ------------------------------------------------------------ Info cmd -----------------------------------------------------------

// info command callback
int cmd_info(int argq, char **argv){
	if(!logged){
		printf("Please login first before executing this priviledged command\n");
		return 0;
	}

	console_print_info();
	return 0;
}

// info command struct 
static const esp_console_cmd_t cmd_info_cmd = {
    .command = "info",
    .help = "Show system data",
    .hint = NULL,
    .func = &cmd_info,
	.argtable = NULL
};

// ------------------------------------------------------------ Register all commands ----------------------------------------------

// register commands
void cmds_register(void){
	logged = false;
	
	// wifi command
	cmd_wifi_args.subcommand    	= arg_str1(NULL, NULL, "<subcommand>", 	"wifi subcommands. Can be: info, disconnect, connect <ssid> [password] [timeout_ms]");
	cmd_wifi_args.ssid    			= arg_str0(NULL, NULL, "<ssid>",    	"wifi ssid to connect to");
	cmd_wifi_args.pass    			= arg_str0(NULL, NULL, "<pass>",    	"wifi passwrod to connect to");
	cmd_wifi_args.timeout   		= arg_int0(NULL, NULL, "<timeout_ms>",  "timeout to wait for a connection");
	cmd_wifi_args.end 				= arg_end(10);
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_wifi_cmd));
	
	// restart
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_restart_cmd));

	// restartinfo
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_info_cmd));

	// login
	cmd_login_args.pass 			= arg_str1(NULL, NULL, "<password>", NULL);
	cmd_login_args.end 				= arg_end(10);
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_login_cmd));

	// exit
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_exit_cmd));
}