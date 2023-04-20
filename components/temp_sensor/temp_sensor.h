#ifndef _TEMP_SENSOR_HEADER_
#define _TEMP_SENSOR_HEADER_

#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

typedef enum{
	temp_err_adc_read_error			= -1,	
	temp_err_adc_cali_driver_error	= -2,	
	temp_err_ok						=  0,	
}temp_err_t;

typedef struct{
	int channel;
	adc_oneshot_unit_handle_t unit;
	adc_cali_handle_t cali;
	int voltage;
	float temperature;
}temp_sensor_t;

/**
 * @brief init solar sensor, adc and all
 */
temp_sensor_t temp_sensor_init(int channel, int unit_id, adc_oneshot_unit_handle_t unit);

/**
 * @brief read data from sensor, voltage and temperature
 */
temp_err_t temp_sensor_read(temp_sensor_t *sensor);

/**
 * @brief get err string
 */
const char *temp_err_to_str(temp_err_t code);

#endif
