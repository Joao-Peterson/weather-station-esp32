#include "rain_gauge.h"
#include <esp_timer.h>
#include <driver/gpio.h>
#include "sdkconfig.h"
#include "enumstr.h"
#include "vars.h"

// errors
const enum_str_t rain_gauge_err_str[] = {
	ENUMSTR_ENTRY(rain_gauge_err_ok)
};

// get err as message
const char *rain_gauge_err_to_str(rain_gauge_err_t code){
	return enumstr_get(rain_gauge_err_str, code);
}

// global counter
static uint32_t hall_counter = 0;

// isr handler
static void IRAM_ATTR hall_isr_handler(void *arg){
	hall_counter++;
}

// sensor init
rain_gauge_t rain_gauge_init(int port){
	// config gpio
    gpio_config_t hall_gpio = { 
		.intr_type = GPIO_INTR_NEGEDGE, 
		.mode = GPIO_MODE_INPUT, 
		.pin_bit_mask = (1ULL<<port) 
	};

	gpio_config(&hall_gpio);

	// add isr callback
	gpio_isr_handler_add(port, hall_isr_handler, NULL);

	rain_gauge_t sensor = {0};
	return sensor;
}

// read
rain_gauge_err_t rain_gauge_read(rain_gauge_t *sensor){
	const var_t *ratio = varGet("rain_r");

	uint32_t cur_count = hall_counter;
	int64_t time = esp_timer_get_time();

	sensor->precipitation_inst = cur_count * ratio->value.asFloat;

	if(time > (sensor->last_minute + 60*1000000ULL)){
		sensor->last_minute = time;	

		if(sensor->last_minute_precipitation > cur_count)
			sensor->last_minute_precipitation = 0;

		sensor->precipitation_mm_min = 
			ratio->value.asFloat * (cur_count - sensor->last_minute_precipitation);

		sensor->last_minute_precipitation = cur_count;
	}

	if(time > (sensor->last_hour + 60*60*1000000ULL)){
		sensor->last_hour = time;

		if(sensor->last_hour_precipitation > cur_count)
			sensor->last_hour_precipitation = 0;

		sensor->precipitation_mm_hour = 
			ratio->value.asFloat * (cur_count - sensor->last_hour_precipitation);

		sensor->last_hour_precipitation = cur_count;
	}

	if(time > (sensor->last_day + 24*60*60*1000000ULL)){
		sensor->last_day = time;

		if(sensor->last_day_precipitation > cur_count)
			sensor->last_day_precipitation = 0;

		sensor->precipitation_mm_min = 
			ratio->value.asFloat * (cur_count - sensor->last_day_precipitation);

		sensor->last_day_precipitation = cur_count;
	}

	return rain_gauge_err_ok;
}

