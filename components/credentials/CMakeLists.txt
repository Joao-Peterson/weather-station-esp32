idf_component_register(
    SRCS "cred.c"
    INCLUDE_DIRS "."
    REQUIRES "nvs_flash esp_system esp_app_format mbedtls hexstring"
)