#include "enumstr.h"

// get string from array of enum_str
char *_enumstr_get(const enum_str_t array[], size_t array_size, int index){
	for(int i = 0; i < array_size; i++){
		if(array[i].value == index)
			return array[i].string;
	}

	return NULL;
}