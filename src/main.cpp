#include <FastLED.h>
#include "QList.h"
#include "ConfigManager.h"
#include "time.h"
#include "ClockFace.hpp"
#include "Wordclock.hpp"
#include "main.hpp"

const int WORDLAYOUT = 1;

int aktuelleMinute;
int aktuelleStunde;

int lastTimeDisplayed;

struct Config
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
} config;

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
} validConfig;

uint8_t currentMinute;
uint8_t currentHour;

uint8_t lastMinute = -1;

bool wifiVerbunden;
bool inAPMode;
bool ntpZuletztVerwendet = false; //relevant für manuelle Zeiteinstellung

extern int countdown2;

struct tm timeinfo;

const char *ntpServer = "fritz.box";
const char *passwort1 = "SOURSEVE";
const char *passwort2 = "SVNEHTNV";
const char *passwort3 = "OSVNRNEV";
const char *apName1 = "WordclockV1";
const char *apName2 = "WordclockV2";
const char *apName3 = "WordclockV3";

const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;



struct Metadata
{
    int8_t version;
} meta;


WebServer server;
DNSServer dnsServer;
Networking net(dnsServer);
ConfigManager configManager(server, net);
ClockFace cf;
Wordclock wc(cf);


void setup ()
{
    Serial.begin (115200);
    Serial.println ("Setup started");

    cf.display_setup ();

    meta.version = 3; //ConfigManagaer

    Serial.println ("Init ConfigManager");
    net.setAPName ((WORDLAYOUT == 1) ? apName1 : (WORDLAYOUT == 2) ? apName2 : apName3);
    net.setAPPassword ((WORDLAYOUT == 1) ? passwort1 : (WORDLAYOUT == 2) ? passwort2 : passwort3);

    configManager.addParameter ("hour", config.hour, 10);
    configManager.addParameter ("minute", config.minute, 10);
    configManager.addParameter ("ntpUse", config.ntpUse, 10);
    configManager.addParameter ("ntpServer", config.ntpServer, 20);
    configManager.addParameter ("c1", config.c1, 10);
    configManager.addParameter ("c2", config.c2, 10);
    configManager.addParameter ("c3", config.c3, 10);
    configManager.addParameter ("c4", config.c4, 10);
    configManager.addParameter ("sat", config.sat, 10);
    configManager.addParameter ("bri", config.bri, 10);
    configManager.addParameter ("enabled", &config.enabled);
    configManager.addParameter ("version", &meta.version, BaseParameter::ParameterMode::get);

    configManager.begin (config);
    server.begin();

    Serial.println ("Init Colors");
    checkConfig (true);
    checkWifi (true);


    lastTimeDisplayed = -1;
}

void loop ()
{
    // Always do...
    configManager.loop ();

    net.loop();
    server.handleClient ();
    dnsServer.processNextRequest ();


    checkConfig (false);
    checkWifi (false);

    wc.loop();

    delay (1);
}

void checkConfig (bool init)
{
    bool changeColor = false;
    bool changeTimeCfg = false;
    bool changeTime = false;

    int tempInt = atoi (config.c1);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (validConfig.c1 != tempInt)
        {
            validConfig.c1 = tempInt;
            changeColor = true;
            Serial.println ("Color 1 changed");
        }
    }
    else
    {
        itoa (validConfig.c1, config.c1, 10);
    }

    tempInt = atoi (config.c2);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (validConfig.c2 != tempInt)
        {
            validConfig.c2 = tempInt;
            changeColor = true;
            Serial.println ("Color 2 changed");
        }
    }
    else
    {
        itoa (validConfig.c2, config.c2, 10);
    }

    tempInt = atoi (config.c3);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (validConfig.c3 != tempInt)
        {
            validConfig.c3 = tempInt;
            changeColor = true;
            Serial.println ("Color 3 changed");
        }
    }
    else
    {
        itoa (validConfig.c3, config.c3, 10);
    }

    tempInt = atoi (config.c4);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (validConfig.c4 != tempInt)
        {
            validConfig.c4 = tempInt;
            changeColor = true;
            Serial.println ("Color 4 changed");
        }
    }
    else
    {
        itoa (validConfig.c4, config.c4, 10);
    }

    tempInt = atoi (config.sat);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (validConfig.sat != tempInt)
        {
            validConfig.sat = tempInt;
            changeColor = true;
            Serial.println ("Saturation changed");
        }
    }
    else
    {
        itoa (validConfig.sat, config.sat, 10);
    }

    tempInt = atoi (config.bri);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (validConfig.bri != tempInt)
        {
            validConfig.bri = tempInt;
            changeColor = true;
            Serial.println ("Brightness changed");
        }
    }
    else
    {
        itoa (validConfig.bri, config.bri, 10);
    }

    bool tempNtpUse = (0U == strcmp (config.ntpUse, "1")) && (net.getMode () == Networking::Mode::API);
    if (validConfig.ntpUse != tempNtpUse)
    {
        validConfig.ntpUse = tempNtpUse;
        changeTimeCfg = true;
        Serial.println ("ntpUse changed");
    }

    if (validConfig.ntpUse)
    {
        if ((0U != strcmp(config.ntpServer, ntpServer)) && (0U != strcmp(config.ntpServer, "")))
        {
            // String changed and is not empty
            ntpServer = config.ntpServer;
            changeTimeCfg = true;
            Serial.println ("ntpServer changed");
        }
    }

    if ((changeColor or changeTimeCfg) && !init)
    {
        Serial.println ("Colors or TimeCfg updated");
        wc.zeigeNachrichtOk ();
        countdown2 = 0;
    }

    if (changeTimeCfg or init)
    {
        Serial.print ("Activate NTP");
        WiFi.config (0U, 0U, 0U);
        delay (5000);
        //configManager.startAPI();
        configTime (gmtOffset_sec, daylightOffset_sec, "fritz.box");
        setenv ("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);
        getLocalTime (&timeinfo);
        ntpZuletztVerwendet = true;
    }

    if (!validConfig.ntpUse)
    {
        tempInt = atoi (config.hour);
        if (tempInt >= 0 && tempInt < 24)
        {
            if (validConfig.hour != tempInt)
            {
                validConfig.hour = tempInt;
                changeTime = true;
                Serial.println ();
                Serial.print ("hour changed to ");
                Serial.print (validConfig.hour);
            }
        }
        else
        {
            itoa (validConfig.hour, config.hour, 10);
        }

        tempInt = atoi (config.minute);
        if (tempInt >= 0 && tempInt < 24)
        {
            if (validConfig.minute != tempInt)
            {
                validConfig.minute = tempInt;
                changeTime = true;
                Serial.println ();
                Serial.print ("minute changed to ");
                Serial.print (validConfig.minute);
            }
        }
        else
        {
            itoa (validConfig.minute, config.minute, 10);
        }
    }

    if (changeColor or init)
    {
        wc.setzeFarben ();
    }

    if (changeTime)
    {
        if (!init)
        {
            wc.zeigeNachrichtOk ();
            countdown2 = 0;
        }
        if (!ntpZuletztVerwendet)
        {
            configTime (0, daylightOffset_sec, ntpServer);
        }
        getLocalTime (&timeinfo);
        long currentTime = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60;
        if (ntpZuletztVerwendet)
        {
            currentTime -= daylightOffset_sec;
        }
        long newTime = validConfig.hour * 3600 + validConfig.minute * 60;

        configTime (newTime - currentTime, daylightOffset_sec, ntpServer);
        ntpZuletztVerwendet = false;
    }
}

void checkWifi (bool init)
{
    bool inAPModeAktuell = net.getMode () == Networking::Mode::AP;
    bool wifiAktuell = WiFi.status () == WL_CONNECTED;
    if (init or wifiAktuell != wifiVerbunden or inAPModeAktuell != inAPMode)
    {
        wifiVerbunden = wifiAktuell;
        inAPMode = inAPModeAktuell;
        cf.bearbeiteListe (7);
        if (inAPMode)
        {
            cf.reiheWortWifiInListe (3, WORDLAYOUT);
            cf.bearbeiteListe (4);
            delay (2000);
            cf.bearbeiteListe (6);
            wc.zeigePasswort ();
            IPAddress local = WiFi.softAPIP ();
            wc.zeigeIPAdresse (local, 0, 3);
        }
        else if (wifiVerbunden)
        {
            cf.reiheWortOnlineInListe (3, WORDLAYOUT);
            cf.bearbeiteListe (4);
            delay (2000);
            cf.bearbeiteListe (6);
            IPAddress local = WiFi.localIP ();
            wc.zeigeIPAdresse (local, 3, 3);
        }
        else if (!wifiVerbunden)
        {
            cf.reiheWortOfflineInListe (3, WORDLAYOUT);
            cf.bearbeiteListe (5);
            delay (2000);
            cf.bearbeiteListe (6);
        }
        cf.bearbeiteListe (8);
    }
}


