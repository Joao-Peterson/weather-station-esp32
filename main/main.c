#include <stdio.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_vfs_dev.h>
#include <esp_vfs_fat.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_app_desc.h>
#include <esp_console.h>
#include <esp_timer.h>
#include <linenoise/linenoise.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_adc/adc_continuous.h>
#include "cmds.h"
#include "wifi.h"
#include "queue.h"
#include "vars.h"
#include "hg_sensor.h"
#include "solar_sensor.h"
#include "temp_sensor.h"
#include "rain_gauge.h"

// ------------------------------------------------------------ Defines ------------------------------------------------------------

#define PROMPT_STR CONFIG_IDF_TARGET

// ------------------------------------------------------------ Globals ------------------------------------------------------------

// ------------------------------------------------------------ Prototypes ---------------------------------------------------------

// console_init
void console_init(void);

// main console task
void console_task(void *data);

// main console task
void sensors_task(void *data);

// ------------------------------------------------------------ Main ---------------------------------------------------------------

void app_main(void){
    // initialize flash
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // initialize wifi
    wifi_init();

    // run console task
    xTaskCreate(console_task, "console_task", 4096, NULL, 0, NULL);

    // sensor task
    xTaskCreate(sensors_task, "sensors_task", 4096, NULL, 0, NULL);   
}

// ------------------------------------------------------------ Sensors task -------------------------------------------------------

// main console task
void sensors_task(void *data){

    // sensors init
    // rain_gauge_t rain_data;

    int unit_id = CONFIG_ANALOG_SENSORS_ADC_UNIT;

    // init adc1 unit 
	adc_oneshot_unit_init_cfg_t unit_cfg = {
		.unit_id = unit_id,
		.ulp_mode = ADC_ULP_MODE_DISABLE
	};
	
    adc_oneshot_unit_handle_t adc_unit0_handle;
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_unit0_handle));

    hg_sensor_t hg_sensor           = hg_init(CONFIG_GPIO_HUMIDITY);
    solar_sensor_t solar_sensor     = solar_sensor_init(CONFIG_SOLAR_CELL_ADC_CHANNEL, unit_id, adc_unit0_handle);
    temp_sensor_t temp_sensor       = temp_sensor_init(CONFIG_TEMP_SENSOR_ADC_CHANNEL, unit_id, adc_unit0_handle);

    // gpio_config_t hall_gpio         = { .intr_type = GPIO_INTR_ANYEDGE, .mode = GPIO_MODE_INPUT, .pin_bit_mask = (1ULL<<CONFIG_GPIO_HALL_SENSOR) };
    // gpio_config(&adc0_gpio);
    // gpio_config(&hall_gpio);
    // gpio_config(&solar_incidence);
    
    gpio_install_isr_service(0);

    // raingauge
    // rain_data.ratio_counts_per_mm = RAIN_GAUGE_RATIO_COUNTS_PER_MM; // rain gauge initialization information
    // rain_data.counts = 0;
    // rain_data.precipitation_inst = 0.0;
    // global_rain_data = &rain_data;

    // hall_sensor_tmp = gpio_get_level(CONFIG_GPIO_HALL_SENSOR);      // save hall state
    int64_t last_call = esp_timer_get_time();                       // get time for calling humidity sensor

    // task queues
    // queue_double_t *temp_queue = queue_new(50);
    // queue_double_t *solar_queue = queue_new(30);

    // main loop
    while(1){

        /* Temperature */
        // uint16_t temp_raw  = adc1_get_raw(ADC1_CHANNEL_0);
        // float temp_voltage = (temp_raw*3.3/4096);
        // float temp         = (temp_voltage*100.0);
        // queue_add(temp_queue, (double)temp);
        // objeto.temp        = queue_average(temp_queue);
        // // ESP_LOGI(__FILE__, "Temperatura: [%f]", temp);
        
        /* Humidity */  
        if(esp_timer_get_time() - last_call > HG_SENSOR_COOLDOWN_TIME_US){
            hg_err_t hg_code = hg_read(&hg_sensor);

            // if error has occured
            if(hg_code){
                ESP_LOGE(__FILE__, "%s", hg_err_to_str(hg_code));
                sensor_data.hg_temp     = NAN;
                sensor_data.hg_humidity = NAN;
            }
            else{
                sensor_data.hg_temp     = hg_sensor.temperature;
                sensor_data.hg_humidity = hg_sensor.humidity;
            }
            
            last_call = esp_timer_get_time();
        }

        /* Solar incidence */
        solar_err_t solar_code = solar_sensor_read(&solar_sensor);

        if(solar_code){
            ESP_LOGE(__FILE__, "%s", solar_err_to_str(solar_code));
            sensor_data.solar_incidency = NAN;
            sensor_data.solar_voltage = NAN;
        }
        else{
            sensor_data.solar_incidency = solar_sensor.incidency;
            sensor_data.solar_voltage = solar_sensor.voltage;
        }

        /* Temperature */
        temp_err_t temp_code = temp_sensor_read(&temp_sensor);

        if(temp_code){
            ESP_LOGE(__FILE__, "%s", temp_err_to_str(temp_code));
            sensor_data.temp_temperature = NAN;
            sensor_data.temp_voltage = NAN;
        }
        else{
            sensor_data.temp_temperature = temp_sensor.temperature;
            sensor_data.temp_voltage = temp_sensor.voltage;
        }

        /* Rain gauge */
        // hall_sensor = gpio_get_level(CONFIG_GPIO_HALL_SENSOR);
        // if(hall_sensor == 0 && hall_sensor_tmp == 1){       // falling edge detection
        //     rain_data.counts++;
        //     // objeto.precipitation = rain_data.counts;
        // }
        // hall_sensor_tmp = hall_sensor;
    }    
}

// ------------------------------------------------------------ Console task -------------------------------------------------------

// console_init
void console_init(void){
    // Drain stdout before reconfiguring it 
    fflush(stdout);
    fsync(fileno(stdout));

    // Disable buffering on stdin 
    setvbuf(stdin, NULL, _IONBF, 0);

    // Minicom, screen, idf_monitor send CR when ENTER key is pressed 
    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    // Move the caret to the beginning of the next line on '\n' 
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

    /** Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */ 
    const uart_config_t uart_config = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,

        #if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
        .source_clk = UART_SCLK_REF_TICK,
        #else
        .source_clk = UART_SCLK_XTAL,
        #endif
    };

    // Install UART driver for interrupt-driven reads and writes 
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    // Tell VFS to use UART driver, r/w operations will be blocking ones
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    // Initialize the console 
    esp_console_config_t console_config = {
        .max_cmdline_args = 25,
        .max_cmdline_length = 256,

        #if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN)
        #endif
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /** Configure linenoise line completion library 
     * Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    // Tell linenoise where to get command completions and hints 
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    // Set command maximum length 
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    // Don't return empty lines 
    linenoiseAllowEmpty(false);
}

// main console task
void console_task(void *data){

    console_init();
    esp_console_register_help_command();
    cmds_register();

    /** Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char *prompt = LOG_COLOR_I PROMPT_STR "> " LOG_RESET_COLOR;

    printf("\n"
        "Weather station console.\n"
        "Use UP/DOWN arrows to navigate through command history.\n"
        "Press TAB when typing command name to auto-complete.\n"
        "Type 'help' to get the list of commands.\n"
        "Log in using the 'login' to use all available commands\n"
    );

    // Figure out if the terminal supports escape sequences 
    int probe_status = linenoiseProbe();
    if (probe_status) { // zero indicates success 
        printf("\n"
            "Your terminal application does not support escape sequences.\n"
            "Line editing and history features are disabled.\n"
            "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);

        #if CONFIG_LOG_COLORS
        /** Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = PROMPT_STR "> ";
        #endif //CONFIG_LOG_COLORS
    }

    // main loop
    while(1){
        // get line after enter
        char *line = linenoise(prompt);

        // on EOF or error
        if(line == NULL){
            continue;
        }        

        // add command to history
        if(strlen(line) > 0){
            linenoiseHistoryAdd(line);
        }

        // execute
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        
        // free
        linenoiseFree(line);
    }    

    esp_console_deinit();
}