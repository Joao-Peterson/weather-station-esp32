#include "temp_sensor.h"
#include "enumstr.h"
#include "sdkconfig.h"
#include <esp_err.h>
#include "vars.h"

// errors
const enum_str_t temp_err_str[] = {
	ENUMSTR_ENTRY(temp_err_adc_read_error),
	ENUMSTR_ENTRY(temp_err_adc_cali_driver_error),
	ENUMSTR_ENTRY(temp_err_ok)
};

// get string
const char *temp_err_to_str(temp_err_t code){
	return enumstr_get(temp_err_str, code);
}

/**
 * The ADC in ESP32 has a maximum input voltage range of 0 to 1.1V.
 * 
 * ADC_ATTEN_DB_0: No attenuation (input voltage range 0-1.1V)
 * ADC_ATTEN_DB_2_5: Attenuation by a factor of 2.5 (input voltage range 0-1.5V)
 * ADC_ATTEN_DB_6: Attenuation by a factor of 6 (input voltage range 0-2.2V)
 * ADC_ATTEN_DB_11: Attenuation by a factor of 11 (input voltage range 0-3.9V)
*/
static const int atten2mv[] = {1100, 1500, 2200, 3900};

static int adc2mv(int raw, adc_bitwidth_t bitwidth, adc_atten_t atten){
	uint32_t res = (1 << (bitwidth ? bitwidth : ADC_BITWIDTH_13)) - 1; 				// max raw adc value
	uint32_t volmax = atten2mv[atten]; 												// adc unit max read voltage
	return (int)(raw * res / volmax);
}

// init solar sensor 
temp_sensor_t temp_sensor_init(int channel, int unit_id, adc_oneshot_unit_handle_t unit){
	temp_sensor_t s = {0};
	s.channel = channel;
	s.unit = unit;
	
	// adc channel config
	adc_oneshot_chan_cfg_t cfg = {
		.bitwidth = CONFIG_SOLAR_CELL_ADC_BITWIDTH,
		.atten = CONFIG_SOLAR_CELL_ADC_ATTEN
	};

	ESP_ERROR_CHECK(adc_oneshot_config_channel(s.unit, s.channel, &cfg));

	// channel calibration
	#if CONFIG_TEMP_ADC_USE_CALI

	adc_cali_line_fitting_config_t cali_cfg = {
		.atten = CONFIG_TEMP_ADC_ATTEN,
		.bitwidth = CONFIG_TEMP_ADC_BITWIDTH,
		.unit_id = unit_id
	};

	ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_cfg, &(s.cali)));
	#endif // CONFIG_TEMP_ADC_USE_CALI

	return s;
}

// read voltage for the sensor
temp_err_t temp_sensor_read(temp_sensor_t *sensor){

	if(adc_oneshot_read(sensor->unit, sensor->channel, &(sensor->voltage))) 		// read raw
		return temp_err_adc_read_error;
	
	if(CONFIG_TEMP_ADC_USE_CALI && (sensor->cali != NULL)){							// get voltage with the cali driver
		if(adc_cali_raw_to_voltage(sensor->cali, sensor->voltage, &(sensor->voltage)))
			return temp_err_adc_cali_driver_error;
	}
	else																			// get voltage by linear proportion
		sensor->voltage = adc2mv(sensor->voltage, CONFIG_TEMP_ADC_BITWIDTH, CONFIG_TEMP_ADC_ATTEN);		

	const var_t *a = varGet("temp_a");
	const var_t *b = varGet("temp_b");
	sensor->temperature = 															// calculate solar incidency. a * x + b
		a->value.asFloat * sensor->voltage + b->value.asFloat;

	return temp_err_ok;
}
