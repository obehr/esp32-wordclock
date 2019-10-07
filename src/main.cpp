#include <FastLED.h>
#include "QList.h"
#include "ConfigManager.h"

#include "ClockFace.hpp"
#include "Wordclock.hpp"
#include "Config.hpp"
#include "main.hpp"


const int WORDLAYOUT = 1;
const char *passwort1 = "SOURSEVE";
const char *passwort2 = "SVNEHTNV";
const char *passwort3 = "OSVNRNEV";
const char *apName1 = "WordclockV1";
const char *apName2 = "WordclockV2";
const char *apName3 = "WordclockV3";

int aktuelleMinute;
int aktuelleStunde;

WebServer server;
DNSServer dnsServer;
Networking net(dnsServer);
ConfigManager configManager(server, net);
ClockFace cf (
    1,  // Matrix layout
    1   // Orientation
);
Config cfg(configManager);
Wordclock wc(net, cf, cfg);

void setup ()
{
    Serial.begin (115200);
    Serial.println ("Setup started");

    // Load config defaults
    wc.checkConfig (true);
    wc.checkWifi (true);

    cf.display_setup ();
    net.setAPName ((WORDLAYOUT == 1) ? apName1 : (WORDLAYOUT == 2) ? apName2 : apName3);
    net.setAPPassword ((WORDLAYOUT == 1) ? passwort1 : (WORDLAYOUT == 2) ? passwort2 : passwort3);
    configManager.attach(wc);
    configManager.begin (cfg.cfg_raw);
    server.begin();


}

void loop ()
{
    // Always do...
    configManager.loop ();

    net.loop();
    server.handleClient ();
    dnsServer.processNextRequest ();

    wc.loop();
    delay (1);
}



