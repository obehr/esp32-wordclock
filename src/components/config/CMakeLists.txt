idf_component_register(SRCS "config.cpp"
                  INCLUDE_DIRS "."
                  REQUIRES spi_flash nvs_flash json)