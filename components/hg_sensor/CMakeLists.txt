idf_component_register(
    SRCS "hg_sensor.c"
    INCLUDE_DIRS "."
    REQUIRES "freertos log esp_common esp_timer enumstr soc driver"
)