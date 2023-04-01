#ifndef _HG_SENSOR_HEADER_
#define _HG_SENSOR_HEADER_

#include <esp_types.h>
#include <driver/gpio.h>

// ------------------------------------------------------------ Defines ------------------------------------------------------------

#define PACKET_BYTES_QTY 5 

// if kconfig option is defined then its value is used, otherwise has a 500 ms cooldown
#ifdef CONFIG_HUMIDITY_READ_COOLDOWN_MS
    #define HG_SENSOR_COOLDOWN_TIME_US (CONFIG_HUMIDITY_READ_COOLDOWN_MS*1000)
#else
    #define HG_SENSOR_COOLDOWN_TIME_US 500000 
#endif 

// ------------------------------------------------------------ Types --------------------------------------------------------------

/**
 * @brief defines a union for the binary packet over the humidity sensor data structure
 */
#pragma pack(push,1)
typedef union{
    struct{
        uint8_t checksum;
        uint16_t temperature;
        uint16_t humidity;
    };
    uint8_t byte[PACKET_BYTES_QTY];													/**< binary data from the wire */
} hg_packet_t;
#pragma pack(pop)

/**
 * @brief struct for the treated data from the sensor after reading, hg_read() call
 */
typedef struct{
    hg_packet_t raw;
    float humidity;
    float temperature;
}hg_sensor_t;

// ------------------------------------------------------------ Error --------------------------------------------------------------

// enumerator with errors
typedef enum{
    hg_err_timeout                      =  -1, 
    hg_err_pointer_to_null              =  -2, 
    hg_err_fail_to_read_response_bits   =  -3, 
    hg_err_sensor_on_cooldown_time      =  -4, 
    hg_err_ok                           =   0, 
    hg_err_checksum_failed              =   1, 
}hg_err_t; 

// ------------------------------------------------------------ Functions ----------------------------------------------------------

/**
 * @brief return string for error given by the hg_err_t code
 */
const char *hg_err_to_str(hg_err_t code);

/**
 * @brief read data from sensor and return treated values
 * @param port: gpio port to read data from
 * @param sensor_data: allocatted struct to write data into
 * @return hg_err_t type error, use hg_err_str to get a string from the error
 */
hg_err_t hg_read(gpio_num_t port, hg_sensor_t *sensor_data);

#endif