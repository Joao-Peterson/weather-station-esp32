#ifndef _RAIN_GAUGE_HEADER_
#define _RAIN_GAUGE_HEADER_

#include <esp_types.h>
#include <driver/gpio.h>

// ------------------------------------------------------------ Types --------------------------------------------------------------

/**
 * @brief error handling
 */
typedef enum{
    rain_gauge_err_ok               = 0   
}rain_gauge_err_t;


/**
 * @brief struct containg raing gauge data
 */
typedef struct{
    int64_t last_minute;                                                            /**< last time measure for a minute in microseconds */
    int64_t last_hour;                                                              /**< last time measure for a hour in microseconds */
    int64_t last_day;                                                               /**< last time measure for a day in microseconds */
    float precipitation_inst;                                                       /**< precipitation, (counts * ratio_counts_per_mm) */
    uint32_t last_minute_precipitation;                                             /**< last time sensor counter for a minute */
    uint32_t last_hour_precipitation;                                               /**< last time sensor counter for a hour */
    uint32_t last_day_precipitation;                                                /**< last time sensor counter for a day */
    float precipitation_mm_min;                                                     /**< precipitation per minute */
    float precipitation_mm_hour;                                                    /**< precipitation per hour */
    float precipitation_mm_day;                                                     /**< precipitation per day */
}rain_gauge_t;

// ------------------------------------------------------------ Functions ----------------------------------------------------------

/**
 * @brief initialize a new rain gauge. Hanles gpio config
 */
rain_gauge_t rain_gauge_init(int port);

/**
 * @brief read data to rain gauge struct
 */
rain_gauge_err_t rain_gauge_read(rain_gauge_t *sensor);

/**
 * @brief get err as message
 */
const char *rain_gauge_err_to_str(rain_gauge_err_t code);

#endif