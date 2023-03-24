idf_component_register(
    SRCS "towerlight-mail-esp.c"
    INCLUDE_DIRS "."
    REQUIRES "console freertos fatfs nvs_flash esp_system esp_app_format cmds"
)
