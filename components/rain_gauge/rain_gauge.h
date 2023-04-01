#ifndef _RAIN_GAUGE_HEADER_
#define _RAIN_GAUGE_HEADER_

#include <esp_types.h>
#include <driver/gpio.h>

// ------------------------------------------------------------ Defines ------------------------------------------------------------

// if a ratio is defined for precipitation use it, otherwise the default value is 1.0.
// the ration is given by a string
#ifdef CONFIG_RAIN_GAUGE_RATIO_COUNTS_PER_MM
    #define RAIN_GAUGE_RATIO_COUNTS_PER_MM (CONFIG_RAIN_GAUGE_RATIO_COUNTS_PER_MM/1000.0);
#else
    #define RAIN_GAUGE_RATIO_COUNTS_PER_MM 1.0;
#endif

// ------------------------------------------------------------ Types --------------------------------------------------------------

/**
 * @brief struct containg raing gauge data
 */
typedef struct{
    uint32_t counts;                                                                /**< discrete counts from the sensor */
    float ratio_counts_per_mm;                                                      /**< ratio between counts and mm of rain */
    float precipitation_inst;                                                       /**< precipitation, (counts * ratio_counts_per_mm) */
    float precipitation_mm_min;                                                     /**< precipitation per minute */
}rain_gauge_t;

// ------------------------------------------------------------ Functions ----------------------------------------------------------


#endif