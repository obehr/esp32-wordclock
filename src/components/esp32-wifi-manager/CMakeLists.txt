if(IDF_VERSION_MAJOR GREATER_EQUAL 4)
    idf_component_register(SRC_DIRS src
        REQUIRES log nvs_flash mdns wpa_supplicant lwip esp_http_server config
        INCLUDE_DIRS src
        EMBED_FILES src/style.css src/code.js src/index.html src/settings.html src/jquery.min.js src/jquery.validate.min.js )
else()
    set(COMPONENT_SRCDIRS src)
    set(COMPONENT_ADD_INCLUDEDIRS src)
    set(COMPONENT_REQUIRES log nvs_flash mdns wpa_supplicant lwip esp_http_server config)
    set(COMPONENT_EMBED_FILES src/style.css src/code.js src/index.html src/settings.html src/jquery.min.js src/jquery.validate.min.js)
    register_component()
endif()
