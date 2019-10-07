#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <EEPROM.h>
#include <FS.h>

#include <functional>
#include <list>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32) //ESP32
    #include <WiFi.h>
    #include <WebServer.h>
#endif

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
    using WebServer = ESP8266WebServer;
#endif

#include "Networking.hpp"
#include "Subject.hpp"




/**
 * Base Parameter
 */
class BaseParameter {
public:
    virtual ~BaseParameter() {}
    enum ParameterMode {get, set, both};
    virtual ParameterMode getMode() = 0;
    virtual void fromJson(JsonObject *json) = 0;
    virtual void toJson(JsonObject *json) = 0;
};

/**
 * Config Parameter
 */
template<typename T>
class ConfigParameter : public BaseParameter {
public:
    ConfigParameter(const char *name, T *ptr, ParameterMode mode = both, std::function<void(const char*)> cb = NULL) {
        this->name = name;
        this->ptr = ptr;
        this->cb = cb;
        this->mode = mode;
    }

    ParameterMode getMode() {
        return this->mode;
    }

    void fromJson(JsonObject *json) {
        if (json->containsKey(name) && json->is<T>(name)) {
            *ptr = json->get<T>(name);
        }
    }

    void toJson(JsonObject *json) {
        json->set(name, *ptr);

        if (cb) {
            cb(name);
        }
    }

private:
    const char *name;
    T *ptr;
    std::function<void(const char*)> cb;
    ParameterMode mode;
};

/**
 * Config String Parameter
 */
class ConfigStringParameter : public BaseParameter {
public:
    ConfigStringParameter(const char *name, char *ptr, size_t length, ParameterMode mode = both) {
        this->name = name;
        this->ptr = ptr;
        this->length = length;
        this->mode = mode;
    }

    ParameterMode getMode() {
        return this->mode;
    }

    void fromJson(JsonObject *json) {
        if (json->containsKey(name) && json->is<char *>(name)) {
            const char * value = json->get<const char *>(name);

            memset(ptr,'\n',length);
            strncpy(ptr, const_cast<char*>(value), length - 1);
        }
    }

    void toJson(JsonObject *json) {
        json->set(name, ptr);
    }

private:
    const char *name;
    char *ptr;
    size_t length;
    ParameterMode mode;
};

/**
 * Config Manager
 */
class ConfigManager : public Subject
{
public:
    ConfigManager(WebServer& server, Networking& net);

    void loop();

    template<typename T>
    void begin(T &config) {
        this->config = &config;
        this->configSize = sizeof(T);

        EEPROM.begin(CONFIG_OFFSET + this->configSize);

        setup();
    }

    template<typename T>
    void addParameter(const char *name, T *variable) {
        parameters.push_back(new ConfigParameter<T>(name, variable));
    }
    template<typename T>
    void addParameter(const char *name, T *variable, BaseParameter::ParameterMode mode) {
        parameters.push_back(new ConfigParameter<T>(name, variable, mode));
    }
    void addParameter(const char *name, char *variable, size_t size) {
        parameters.push_back(new ConfigStringParameter(name, variable, size));
    }
    void addParameter(const char *name, char *variable, size_t size, BaseParameter::ParameterMode mode) {
        parameters.push_back(new ConfigStringParameter(name, variable, size, mode));
    }
    void save();



private:

    void *config;
    size_t configSize;

    WebServer& server;
    Networking& net;
    std::list<BaseParameter*> parameters;

    static const uint8_t WIFI_OFFSET;
    static const uint8_t CONFIG_OFFSET;

    static const char magicBytes[]     PROGMEM;
    static const char mimeHTML[]       PROGMEM;
    static const char mimeJSON[]       PROGMEM;
    static const char mimePlain[]      PROGMEM;
    static const char configHTMLFile[] PROGMEM;
    static const char apFilename[]     PROGMEM;

    JsonObject &decodeJson(String jsonString);

    std::function<void(WebServer&)> apCallback;
    std::function<void(WebServer&)> apiCallback;

    void setAPCallback(std::function<void(WebServer&)> callback);
    void setAPICallback(std::function<void(WebServer&)> callback);

    void handleAPGet();
    void handleAPPost();
    void handleRESTGet();
    void handleRESTPut();
    void handleJQueryGet();
    void handleJQueryValidateGet();
    void handleNotFound();
    boolean isIp(String str);

    String toStringIP(IPAddress ip);
    void setup();

    void readConfig();
    void writeConfig();

    static void createCustomRoute (WebServer& server);
};

#endif /* __CONFIGMANAGER_H__ */
