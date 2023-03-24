#include "hexstring.h"

static const char chars[] = {
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'a',
	'b',
	'c',
	'd',
	'e',
	'f'
};

char toUppercase(char in){
	if(in > 'a' && in < 'z'){
		return in - ('a' - 'A');
	}
	else{
		return in;
	}
}

char toLowercase(char in){
	if(in > 'A' && in < 'Z'){
		return in - ('A' - 'a');
	}
	else{
		return in;
	}
}

char *bin2hex(uint8_t *buffer, size_t bufferLen, size_t *outLen, bool uppercase){
	if(buffer == NULL ) return NULL;
	if(bufferLen == 0 ) return NULL;

	size_t len = bufferLen * 2;
	if(outLen != NULL) *outLen = len + 1;

	char *hexstr = calloc(sizeof(char), len + 1);
	for(size_t i = 0; i < bufferLen; i++){
		hexstr[2*i]     = chars[(buffer[i] & (uint8_t)0xF0) >> 4];
		hexstr[2*i + 1] = chars[(buffer[i] & (uint8_t)0x0F)];

		if(uppercase){
			hexstr[2*i] = toUppercase(hexstr[2*i]);
			hexstr[2*i + 1] = toUppercase(hexstr[2*i + 1]);
		}
	}

	return hexstr;
}