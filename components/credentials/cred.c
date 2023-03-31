#include "cred.h"
#include <stdbool.h>
#include <esp_system.h>
#include <esp_app_desc.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <mbedtls/sha256.h>
#include "hexstring.h"

// dealloc credentials struct
void credentials_free(credentials_t *cred){
	free(cred->pass);
	free(cred->serial);
	free(cred);
}

// get password from nvs
credentials_t *credentials_get_nvs(void){
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

// check password against nvs hashed one
bool credentials_check_pass(char *pass){
	credentials_t *cred = credentials_get_nvs();									// get credential
	if(cred == NULL) return false;

	unsigned char hashBin[33] = {0};
	char *hash;
	char passSalted[64+36+1] = {0};													// pass + salt (serial)

	strncat(passSalted, pass, 64+36);
	strncat(passSalted, cred->serial, 64+36);
	mbedtls_sha256((const unsigned char *)passSalted, strlen(passSalted), hashBin, 0);
	hash = bin2hex(hashBin, 32, NULL, false);										// to hexstring

	bool res = false;

	if(!strcmp(cred->pass, hash))													// compare hashes
		res = true;

	free(hash);

	return res;
}