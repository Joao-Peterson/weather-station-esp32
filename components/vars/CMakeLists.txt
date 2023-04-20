idf_component_register(
    SRCS "vars_hashed.c"
    INCLUDE_DIRS "."
    REQUIRES "freertos"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/vars_hashed.c
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/hash ${CMAKE_CURRENT_SOURCE_DIR}/vars.c ${CMAKE_CURRENT_SOURCE_DIR}/vars_hashed.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vars.c
)