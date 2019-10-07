
#include <Arduino.h>
#include <HardwareSerial.h>
#include <time.h>
#include "Config.hpp"

struct Metadata
{
    int8_t version;
} meta;

Config::Config(ConfigManager& cm) :
    cm(cm)
{
    meta.version = 3; //ConfigManagaer

    Serial.println ("Init ConfigManager");


    cm.addParameter ("hour", cfg_raw.hour, 10);
    cm.addParameter ("minute", cfg_raw.minute, 10);
    cm.addParameter ("ntpUse", cfg_raw.ntpUse, 10);
    cm.addParameter ("ntpServer", cfg_raw.ntpServer, 20);
    cm.addParameter ("c1", cfg_raw.c1, 10);
    cm.addParameter ("c2", cfg_raw.c2, 10);
    cm.addParameter ("c3", cfg_raw.c3, 10);
    cm.addParameter ("c4", cfg_raw.c4, 10);
    cm.addParameter ("sat", cfg_raw.sat, 10);
    cm.addParameter ("bri", cfg_raw.bri, 10);
    cm.addParameter ("enabled", &cfg_raw.enabled);
    cm.addParameter ("version", &meta.version, BaseParameter::ParameterMode::get);



    Serial.println ("Init Colors");
}



