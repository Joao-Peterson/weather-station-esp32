idf_component_register(
    SRCS "temp_sensor.c"
    INCLUDE_DIRS "."
    REQUIRES "soc driver enumstr esp_adc vars enumstr"
)