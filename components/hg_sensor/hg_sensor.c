#include "hg_sensor.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_types.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <driver/gptimer.h>
#include <driver/gpio.h>
#include <soc/gpio_struct.h>
#include <esp_err.h>
#include "enumstr.h"

// ------------------------------------------------------------ Error --------------------------------------------------------------

// table for error messages
const enum_str_t hg_err_str[] = {
    ENUMSTR_ENTRY(hg_err_timeout),
    ENUMSTR_ENTRY(hg_err_pointer_to_null),
    ENUMSTR_ENTRY(hg_err_fail_to_read_response_bits), 
    ENUMSTR_ENTRY(hg_err_sensor_on_cooldown_time),   
    ENUMSTR_ENTRY(hg_err_ok),
    ENUMSTR_ENTRY(hg_err_checksum_failed)
};

// ------------------------------------------------------------ Functions ----------------------------------------------------------

/**
 * @brief return string for error given by the hg_err_t code
 */
const char *hg_err_to_str(hg_err_t code){
	return enumstr_get(hg_err_str, code);
}

/**
 * @brief read data from sensor and return treated values
 * @param port: gpio port to read data from
 * @param sensor_data: allocatted struct to write data into
 * @return hg_err_t type error, use hg_err_str to get a string from the error
 */
hg_err_t hg_read(gpio_num_t port, hg_sensor_t *sensor_data){
    int64_t micro_s = 0;
    int64_t temp[PACKET_BYTES_QTY*8*2 + 2];
    static int64_t last_time_called;

    if(sensor_data == NULL){                   
        return hg_err_pointer_to_null;                          					// pointer to NULL struct
    }
    else if(esp_timer_get_time() - last_time_called < HG_SENSOR_COOLDOWN_TIME_US){     
        return hg_err_sensor_on_cooldown_time;                  					// each call should have a 1 sec time interval
    }

    vTaskSuspendAll(); // prevent other tasks
    // send a high-low-high signal to sensor to send data
    // set low for 1 ms
    gpio_set_level(port,0);
    micro_s = esp_timer_get_time();
    while(esp_timer_get_time() < micro_s + 1000);
    // set high for 40 us
    micro_s = esp_timer_get_time();
    gpio_set_level(port,1);
    while(esp_timer_get_time() < micro_s + 40);
    // wait for the sensor low time (80 us), until it pulls the bus high (for another 80 us)
    micro_s = esp_timer_get_time();
    while(gpio_get_level(port) != 1){ 
        if( (esp_timer_get_time() - micro_s) > 80 ){ // no response from sensor 
            xTaskResumeAll(); // since logging is handled by the rtos, we should give it control back by the rtos task to act, by resuming all tasks
            return hg_err_timeout;
        }
    }

    micro_s = esp_timer_get_time();

    // grab times for each transition
    for(int i = 0; i < (PACKET_BYTES_QTY*8*2 + 2); i += 2){
        // there are 5 incoming bytes, therefore a maximum of , 40 high bits that account for 50+70 us each, 
        // 120us*40bits = 4800 us to be waited for all the bits to come ~ 7000 us
        while(gpio_get_level(port) != 0){ if( (esp_timer_get_time() - micro_s) > 7000 ){ xTaskResumeAll(); return hg_err_fail_to_read_response_bits;}}
        temp[i + 0] = esp_timer_get_time();
        
        while(gpio_get_level(port) != 1){ if( (esp_timer_get_time() - micro_s) > 7000 ){ xTaskResumeAll(); return hg_err_fail_to_read_response_bits;}}
        temp[i + 1] = esp_timer_get_time();
    }

    xTaskResumeAll();

    // for every packet
    for(int i = 0; i < PACKET_BYTES_QTY; i++){
        sensor_data->raw.byte[PACKET_BYTES_QTY-1-i] = 0;

        // for every bit in a byte
        for(int j = 0; j < 8; j++){
            // if high time is bigger than the low time then it is a high bit, else is a low bit
            int bit = (temp[2*(j+i*8) + 2] - temp[2*(j+i*8) + 1]) > ( temp[2*(j+i*8) + 1] - temp[2*(j+i*8) + 0]);
            sensor_data->raw.byte[PACKET_BYTES_QTY-1-i] |= (bit<<(8-1-j)); 
        }
    }

    //calculate checksum
    uint16_t checksum = 0;
    for(uint8_t i = PACKET_BYTES_QTY-1; i > 0; i--){
        checksum += sensor_data->raw.byte[i];
    }

    //errors
    if((checksum & 0xFF) != sensor_data->raw.checksum){
        return hg_err_checksum_failed;
        // ESP_LOGW("Humidity sensor", "Humidity sensor checksum failed! [%04X] - [%02X]", checksum, sensor_data->raw.checksum);    
    }

    sensor_data->humidity = sensor_data->raw.humidity / 10.0;

    if(sensor_data->raw.temperature & 0x8000) // if last bit is one then is a negative temperature
        sensor_data->temperature = -(sensor_data->raw.temperature & (~0x8000)) / 10.0;
    else
        sensor_data->temperature = sensor_data->raw.temperature / 10.0;

    last_time_called = esp_timer_get_time();
    return hg_err_ok;
}
