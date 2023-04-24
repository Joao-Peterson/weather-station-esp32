idf_component_register(
    SRCS "mymqtt.c"
    INCLUDE_DIRS "."
    REQUIRES "mqtt freertos credentials esp_wifi log wifi"
    EMBED_TXTFILES "./mosquitto-arch-server.crt"
)