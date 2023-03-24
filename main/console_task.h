#ifndef _CONSOLE_TASK_HEADER_
#define _CONSOLE_TASK_HEADER_

#include <stdio.h>
#include <esp_vfs_dev.h>
#include <esp_vfs_fat.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_app_desc.h>
#include <driver/uart.h>
#include <esp_console.h>
#include <linenoise/linenoise.h>
#include "cmds.h"

#define PROMPT_STR CONFIG_IDF_TARGET

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
        "Log in using the 'login' command before using any commands\n"
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

#endif