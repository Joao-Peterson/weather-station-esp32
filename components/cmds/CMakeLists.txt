idf_component_register(
    SRCS "cmds.c"
    INCLUDE_DIRS "."
    REQUIRES "console freertos fatfs nvs_flash esp_system esp_app_format mbedtls hexstring"
)