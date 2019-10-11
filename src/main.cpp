
/*
 * Note: Define Eclipse build variable ${PLATFORMIO}.
 * - Go to Window -> Preferences -> C/C++ -> Build -> Build variables
 * - Add new variable PLATFORMIO of type String.
 * - For Linux, the default path is ${HOME}/.platformio
 * - For Windows, the default path is ${USERPROFILE}/.platformio
 */

#include "SerialInit.hpp"
#include "ConfigManager.h"
#include "ClockFace.hpp"
#include "Wordclock.hpp"
#include "Config.hpp"
#include "main.hpp"

static const uint8_t WORDLAYOUT   = 1;
static const uint8_t MATRIXLAYOUT = 1;
static const uint8_t ORIENTATION  = 1;

// Create instances of all needed objects
// Constructors will be run through before main setup() function.
SerialInit      si(115200U);
WebServer       server;
DNSServer       dnsServer;
Networking      net(dnsServer);
ConfigManager   configManager(server, net);
Config          cfg(configManager);
ClockFace       cf(MATRIXLAYOUT, ORIENTATION);
Wordclock       wc(net, cf, cfg, WORDLAYOUT);

void setup ()
{
    Serial.println ("Start main setup.");

    // Load config defaults
    wc.checkConfig (true);
    wc.checkWifi (true);

    // Init display
    cf.display_setup ();

    // Init networking
    const char *passwort1 = "SOURSEVE";
    const char *passwort2 = "SVNEHTNV";
    const char *passwort3 = "OSVNRNEV";
    const char *apName1 = "WordclockV1";
    const char *apName2 = "WordclockV2";
    const char *apName3 = "WordclockV3";

    net.setAPName ((WORDLAYOUT == 1) ? apName1 : (WORDLAYOUT == 2) ? apName2 : apName3);
    net.setAPPassword ((WORDLAYOUT == 1) ? passwort1 : (WORDLAYOUT == 2) ? passwort2 : passwort3);

    // Attach Wordclock as observer to observed subject ConfigManager
    configManager.attach(wc);

    // Load config from EEPROM
    configManager.begin (cfg.cfg_raw);

    // Init web server
    server.begin();
}

void loop ()
{
    // ConfigManager loop
    configManager.loop ();

    // networking loop
    net.loop();

    // Handle web server requests
    server.handleClient ();

    // Handle DNS requests
    dnsServer.processNextRequest ();

    // Wordclock loop
    wc.loop();

    // Idle 1 ms to reduce power consumption
    delay (1);
}



