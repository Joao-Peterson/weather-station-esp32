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
	// restart
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_restart_cmd));

	// login
	cmd_login_args.pass = arg_str1(NULL, NULL, "<password>", NULL);
	cmd_login_args.end = arg_end(10);
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_login_cmd));
}