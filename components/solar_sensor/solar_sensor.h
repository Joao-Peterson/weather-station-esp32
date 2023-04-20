#ifndef _SOLAR_SENSOR_HEADER_
#define _SOLAR_SENSOR_HEADER_

#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

typedef enum{
	solar_err_adc_read_error			= -1,	
	solar_err_adc_cali_driver_error		= -2,	
	solar_err_ok						=  0,	
}solar_err_t;

typedef struct{
	int channel;
	adc_oneshot_unit_handle_t unit;
	adc_cali_handle_t cali;
	int voltage;
	float incidency;
}solar_sensor_t;

/**
 * @brief init solar sensor, adc and all
 */
solar_sensor_t solar_sensor_init(int channel, int unit_id, adc_oneshot_unit_handle_t unit);

/**
 * @brief read data from sensor, voltage and incidency
 */
solar_err_t solar_sensor_read(solar_sensor_t *sensor);

/**
 * @brief get err string
 */
const char *solar_err_to_str(solar_err_t code);

#endif
