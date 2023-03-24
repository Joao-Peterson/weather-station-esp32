#ifndef _HEXSTRING_HEADER_
#define _HEXSTRING_HEADER_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

char *bin2hex(uint8_t *buffer, size_t bufferLen, size_t *outLen, bool uppercase);

#endif