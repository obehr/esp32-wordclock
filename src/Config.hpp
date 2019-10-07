
#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <cstdint>
#include <WString.h>
#include "ConfigManager.h"

// Word clock config
class Config
{
public:

    Config (ConfigManager& cm);

    /**
     * Raw config values to/from ConfigManager.
     */
    struct CMConfig
    {
        bool enabled;
        char ntpServer[20];
        char ntpUse[10];
        char hour[10];
        char minute[10];
        char c1[10];
        char c2[10];
        char c3[10];
        char c4[10];
        char bri[10];
        char sat[10];
    } cfg_raw;

    /**
     *
     */
    struct castedConfig
    {
        bool ntpUse = true;
        String ntpServer = "fritz.box";
        uint8_t hour = 0;
        uint8_t minute = 0;
        uint16_t c1 = 50;
        uint16_t c2 = 100;
        uint16_t c3 = 150;
        uint16_t c4 = 200;
        uint16_t bri = 100;
        uint16_t sat = 255;
    } cfg_ok;


private:
    ConfigManager& cm;
};


#endif /* SRC_CONFIG_HPP_ */
