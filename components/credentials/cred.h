#ifndef _CRED_HEADER_
#define _CRED_HEADER_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// store credentials read from memory
typedef struct{
	char *serial;
	char *pass;
}credentials_t;

// dealloc credentials struct
void credentials_free(credentials_t *cred);

// get credentials from nvs
credentials_t *credentials_get_nvs(void);

// check password against nvs hashed one
bool credentials_check_pass(char *pass);

#endif