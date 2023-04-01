idf_component_register(
    SRCS "rain_gauge.c"
    INCLUDE_DIRS "."
    REQUIRES "freertos log esp_common esp_timer enumstr soc driver"
)