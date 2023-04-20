idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES "console freertos fatfs nvs_flash esp_system esp_adc esp_app_format cmds wifi credentials hg_sensor solar_sensor temp_sensor soc driver rain_gauge queue vars"
)
