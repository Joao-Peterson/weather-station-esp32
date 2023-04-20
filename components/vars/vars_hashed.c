#include "vars.h"
#include "sdkconfig.h"
#include "djb2.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// ------------------------------------------------------------ Globals ------------------------------------------------------------

// semaphore for accessing the sensors data 
SemaphoreHandle_t data_sem;

// holds global data from all sensors
sensor_data_t sensor_data;

// ------------------------------------------------------------ Vars data ----------------------------------------------------------

//TODO save vars to flash

// vars
var_t vars[] = {
	// solar incidency corrections
	{.name = "solar_a", .hash = 2733571750, .type = var_float, .value.asFloat = CONFIG_SOLAR_CELL_ADC_INCIDENCY_A 	/ 1000000.0},
	{.name = "solar_b", .hash = 2733571751, .type = var_float, .value.asFloat = CONFIG_SOLAR_CELL_ADC_INCIDENCY_B 	/ 1000000.0},
	{.name = "temp_a", 	.hash = 500614907, 	.type = var_float, .value.asFloat = CONFIG_TEMP_CORR_A 					/ 1000000.0},
	{.name = "temp_b", 	.hash = 500614908, 	.type = var_float, .value.asFloat = CONFIG_TEMP_CORR_B 					/ 1000000.0},
};

// get
const var_t *varGet(const char *name){
	unsigned long hash = djb2_hash((const unsigned char*)name);
	int size = sizeof(vars)/sizeof(var_t);

	for(int i = 0; i < size; i++){
		if(vars[i].hash == hash)
			return &(vars[i]);	
	}

	return NULL;
}

// set
bool varSet(var_t *var, char *value){
	if(
		(var == NULL) || 
		(value == NULL) 
	)
		return false;
	
	switch(var->type){
		case var_const_string:
			var->value.asString = value;
		break;

		case var_string:
			free(var->value.asString);
			var->value.asString = value;
		break;
		
		case var_int:
			var->value.asInt = atoi(value);
		break;
		
		case var_float:
			var->value.asFloat = (float)atof(value);
		break;
		
		case var_bool:
			var->value.asBool = (bool)strcmp(value, "false");
		break;
	}

	return true;
}

// print var
void varPrint(const var_t *var){
	switch(var->type){
		case var_const_string:
		case var_string:
			printf("var '%s' (%s): '%s'\n", var->name, "string", var->value.asString);
		break;
		case var_int:
			printf("var '%s' (%s): %d\n", var->name, "int", var->value.asInt);
		break;
		case var_float:
			printf("var '%s' (%s): % #.5g\n", var->name, "float", var->value.asFloat);
		break;
		case var_bool:
			printf("var '%s' (%s): %s\n", var->name, "bool", var->value.asBool ? "true" : "false");
		break;
	}
}

// print vars
void varsPrint(void){
	int size = sizeof(vars)/sizeof(var_t);

	printf("Available vars: \n");

	for(int i = 0; i < size; i++){
		printf("\t");
		varPrint(&(vars[i]));
	}
}