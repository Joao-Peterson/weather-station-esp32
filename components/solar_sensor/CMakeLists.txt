idf_component_register(
    SRCS "solar_sensor.c"
    INCLUDE_DIRS "."
    REQUIRES "soc driver enumstr esp_adc vars enumstr"
)