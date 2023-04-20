#ifndef _VARS_HEADER_
#define _VARS_HEADER_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ------------------------------------------------------------ Globals ------------------------------------------------------------

// holds global data from all sensors
typedef struct{
    float hg_temp;
    float hg_humidity;
    float solar_incidency;
    float solar_voltage;
    float temp_temperature;
    float temp_voltage;
}sensor_data_t;
extern sensor_data_t sensor_data;

// semaphore for accessing the sensors data 
extern SemaphoreHandle_t data_sem;

// ------------------------------------------------------------ Vars ---------------------------------------------------------------

// var type
typedef enum{
	var_const_string,
	var_string,
	var_int,
	var_float,
	var_bool
}var_type_t;

// var struct
typedef struct{
	const char *name;
	uint32_t hash;
	var_type_t type;
	union{
		char *asString;
		int asInt;
		float asFloat;
		bool asBool;
	}value; 
}var_t;

/**
 * @brief get pointer to variable
 */
const var_t *varGet(const char *name);

/**
 * @brief set the value of a var
 * @param value: value as string format
 */
bool varSet(var_t *var, char *value);

/**
 * @brief print var value
 */
void varPrint(const var_t *var);

/**
 * @brief list all vars
 */
void varsPrint(void);

#endif