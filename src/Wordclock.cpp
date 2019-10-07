
#include <time.h>
#include <HardwareSerial.h>
#include "Wordclock.hpp"
#include "ClockFace.hpp"

struct tm timeinfo;


String wordsMinute[12] =
    { "oclock", "five past", "ten past", "fifteen past", "twenty past", "twentyfive past", "half past", "twentyfive to",
                    "twenty to", "fifteen to", "ten to", "five to" };
String wordsHour[12] =
    { "twelve", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven" };

int letzteMinute;
int letzteStunde;

const int WORDLAYOUT = 1;

uint16_t countdown;
uint8_t countdown2;

Wordclock::Wordclock(Networking& net, ClockFace& cf, Config& cfg) :
    net(net),
    cf(cf),
    cfg(cfg),
    lastTimeDisplayed(-1),
    currentHour(0),
    currentMinute(0),
    lastMinute(-1),
    wifiVerbunden(false),
    inAPMode(false),
    ntpZuletztVerwendet(false)
{
    letzteStunde = -1;
    letzteMinute = -1;
    countdown = 0;
    countdown2 = 0;
}

void Wordclock::notify()
{
    this->checkConfig(false);
    this->checkWifi(false);
}

void Wordclock::loop()
{
    // This still needs to be in the Wordclock loop. Leave it here for the moment.
    // Both methods need to be reworked anyway.
    this->checkConfig(false);
    this->checkWifi(false);

    // Periodically
    // a) Flash LEDs every ~ 1*countdown milliseconds
    // b) Update time every ~ 1*countdown*countdown2 milliseconds
    if (countdown > 0)
    {
        countdown--;
    }
    else
    {
        // Countdown1 is zero

        // Reload
        countdown = 10000;

        if (countdown2 > 0)
        {
            // Countdown2 is not zero --> flash LEDs
            Serial.println ("Flash LEDs");
            countdown2--;
            cf.bearbeiteListe (3);
        }
        else
        {
            // Countdown2 is zero --> Update time

            // Reload
            countdown2 = 10;

            Serial.println ();
            Serial.print ("Updating time...");
            getLocalTime (&timeinfo);
            Serial.print ("Done");

            uint8_t internalHour = timeinfo.tm_hour;
            uint8_t internalMinute = timeinfo.tm_min;
            uint8_t tempHour = internalHour % 12;
            uint8_t tempMinute = (internalMinute + 2) % 60 / 5;

            if (internalMinute > 32)
            {
                tempHour = (tempHour + 1) % 12;
            }

            Serial.println ();
            Serial.print ("Current time: ");
            Serial.print (internalHour);
            Serial.print (":");
            Serial.print (internalMinute);
            Serial.print (":");
            Serial.print (timeinfo.tm_sec);

            if (tempHour != letzteStunde or tempMinute != letzteMinute)
            {
                Serial.println ();
                Serial.print ("Stelle Uhrzeit um von Stunde ");
                Serial.print (letzteStunde);
                Serial.print (" auf ");
                Serial.print (tempHour);
                Serial.print (" und Minute ");
                Serial.print (letzteMinute);
                Serial.print (" auf ");
                Serial.print (tempMinute);
                Serial.println ();
                Serial.print ("In Worten: its ");
                Serial.print (wordsMinute[tempMinute]);
                Serial.print (" (");
                Serial.print (tempMinute);
                Serial.print (") ");
                Serial.print (wordsHour[tempHour]);
                Serial.print (" (");
                Serial.print (tempHour);
                Serial.print (") ");

                cf.clearLists (-1);

                if (letzteStunde != -1)
                {
                    cf.reiheWortItsInListe (0); //kein Ausblenden wenn vorher keine valide Zeit angezeigt wurde
                }
                cf.reiheWortItsInListe (1);
                cf.reiheWortItsInListe (2);
                cf.reiheDummiesInListe (3, 2);

                if (letzteMinute > 0)
                {
                    cf.reiheMinutenInListe (letzteMinute, 0, WORDLAYOUT);
                    cf.reihePastOderToInListe (letzteMinute, 0, WORDLAYOUT);
                }

                if (tempMinute > 0)
                {
                    cf.reiheMinutenInListe (tempMinute, 1, WORDLAYOUT);
                    cf.reiheMinutenInListe (tempMinute, 2, WORDLAYOUT);
                    cf.reiheDummiesInListe (3, 2);
                    cf.reihePastOderToInListe (tempMinute, 1, WORDLAYOUT);
                    cf.reihePastOderToInListe (tempMinute, 2, WORDLAYOUT);
                    cf.reiheDummiesInListe (3, 2);
                }

                if (letzteStunde != -1)
                {
                    cf.reiheStundenInListe (letzteStunde, 0, WORDLAYOUT);
                }

                if (tempHour != -1)
                {
                    cf.reiheStundenInListe (tempHour, 1, WORDLAYOUT);
                    cf.reiheStundenInListe (tempHour, 2, WORDLAYOUT);
                    cf.reiheDummiesInListe (3, 2);
                }

                if (letzteMinute == 0)
                {
                    cf.reiheWortOclockInListe (0, WORDLAYOUT);
                }

                if (tempMinute == 0)
                {
                    cf.reiheWortOclockInListe (1, WORDLAYOUT);
                    cf.reiheWortOclockInListe (2, WORDLAYOUT);
                }

                cf.bearbeiteListe (2);
                letzteStunde = tempHour;
                letzteMinute = tempMinute;
            }
            else
            {
                Serial.println ("Keine Veränderung der Zeitanzeige");
                cf.bearbeiteListe (3);
            }

            Serial.println ("--------------");
            Serial.println ();
        }
    }
}

void Wordclock::setzeFarben ()
{
    Serial.println ();
    Serial.print ("Setze Farben: ");
    Serial.print (cfg.cfg_ok.c1);
    Serial.print (", ");
    Serial.print (cfg.cfg_ok.c2);
    Serial.print (", ");
    Serial.print (cfg.cfg_ok.c3);
    Serial.print (", ");
    Serial.print (cfg.cfg_ok.c4);

    cf.reiheWortItsInListe (1);
    cf.setzeFarbe (cfg.cfg_ok.c1);
    for (int i = 1; i < 12; i++)
    {
        cf.reiheMinutenInListe (i, 1, WORDLAYOUT);
    }
    cf.setzeFarbe (cfg.cfg_ok.c2);
    for (int i = 0; i < 12; i++)
    {
        cf.reiheStundenInListe (i, 1, WORDLAYOUT);
    }
    cf.setzeFarbe (cfg.cfg_ok.c4);
    cf.reiheWortPastInListe (1, WORDLAYOUT);
    cf.setzeFarbe (cfg.cfg_ok.c3);
    cf.reiheWortToInListe (1, WORDLAYOUT);
    cf.setzeFarbe (cfg.cfg_ok.c3);
    cf.reiheWortOclockInListe (1, WORDLAYOUT);
    cf.setzeFarbe (cfg.cfg_ok.c1);
}

void Wordclock::zeigeNachrichtOk ()
{
    cf.bearbeiteListe (7);
    int offsetLayout = 0;
    if (WORDLAYOUT == 2)
    {
        offsetLayout = 2;
    }

    int wortOk[2][2] =
        { 5 - offsetLayout, 7, 7 - offsetLayout, 7 };
    cf.reiheLedsInListe (wortOk, 2, 3, WORDLAYOUT);
    cf.bearbeiteListe (4);
    delay (2000);
    cf.bearbeiteListe (6);
    cf.bearbeiteListe (8);
}

void Wordclock::zeigePasswort ()
{
    cf.bearbeiteListe (7);
    int offsetLayout = 0;
    if (WORDLAYOUT == 3)
    {
        offsetLayout = 1;
    }

    int wortPW[2][2] =
        { 4, 2, 4 + offsetLayout, 3, };
    cf.reiheLedsInListe (wortPW, 2, 3, WORDLAYOUT);
    cf.bearbeiteListe (5);
    delay (1000);
    cf.clearLists (3);

    int wortSechsteZeile[10][2] =
        { 0, 5, 1, 5, 2, 5, 3, 5, 4, 5, 5, 5, 6, 5, 7, 5 };
    cf.reiheLedsInListe (wortSechsteZeile, 8, 3, WORDLAYOUT);
    cf.bearbeiteListe (4);
    delay (10000);
    cf.reiheLedsInListe (wortPW, 8, 3, WORDLAYOUT);
    cf.bearbeiteListe (6);
    cf.bearbeiteListe (8);
}

void Wordclock::zeigeIPAdresse (IPAddress ip, int startOktett, int endeOktett)
{
    Serial.println ();
    Serial.print ("Zeige IP Adresse auf Uhr: ");
    Serial.print (ip);

    int wortIP[2][2] =
        { 4, 1, 4, 2 };
    cf.reiheLedsInListe (wortIP, 2, 3, WORDLAYOUT);
    cf.bearbeiteListe (5);
    delay (2000);
    cf.bearbeiteListe (6);

    for (int i = startOktett; i <= endeOktett; i++)
    {
        Serial.println ();
        uint8_t oktettInt = ip[i];
        Serial.print (oktettInt);
        char oktettChar[3];
        itoa (oktettInt, oktettChar, 10);
        for (int j = 0; j < 3; j++)
        {
            Serial.print (" -> ");
            int ziffer = oktettChar[j] - '0';
            Serial.print (ziffer);
            if (ziffer > 0 && ziffer <= 9)
            {
                cf.reiheStundenInListe (ziffer, 3, WORDLAYOUT);
                cf.bearbeiteListe (4);
                delay (500);
                cf.bearbeiteListe (6);
            }
            else if (ziffer == 0)
            {
                cf.reiheStundenInListe (10, 3, WORDLAYOUT);
                cf.bearbeiteListe (4);
                delay (500);
                cf.bearbeiteListe (6);
            }
            else
            {
                break;  //Ende der Zahl
            }
        }
    }
}

void Wordclock::checkConfig (bool init)
{
    bool changeColor = false;
    bool changeTimeCfg = false;
    bool changeTime = false;

    int tempInt = atoi (cfg.cfg_raw.c1);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (cfg.cfg_ok.c1 != tempInt)
        {
            cfg.cfg_ok.c1 = tempInt;
            changeColor = true;
            Serial.println ("Color 1 changed");
        }
    }
    else
    {
        itoa (cfg.cfg_ok.c1, cfg.cfg_raw.c1, 10);
    }

    tempInt = atoi (cfg.cfg_raw.c2);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (cfg.cfg_ok.c2 != tempInt)
        {
            cfg.cfg_ok.c2 = tempInt;
            changeColor = true;
            Serial.println ("Color 2 changed");
        }
    }
    else
    {
        itoa (cfg.cfg_ok.c2, cfg.cfg_raw.c2, 10);
    }

    tempInt = atoi (cfg.cfg_raw.c3);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (cfg.cfg_ok.c3 != tempInt)
        {
            cfg.cfg_ok.c3 = tempInt;
            changeColor = true;
            Serial.println ("Color 3 changed");
        }
    }
    else
    {
        itoa (cfg.cfg_ok.c3, cfg.cfg_raw.c3, 10);
    }

    tempInt = atoi (cfg.cfg_raw.c4);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (cfg.cfg_ok.c4 != tempInt)
        {
            cfg.cfg_ok.c4 = tempInt;
            changeColor = true;
            Serial.println ("Color 4 changed");
        }
    }
    else
    {
        itoa (cfg.cfg_ok.c4, cfg.cfg_raw.c4, 10);
    }

    tempInt = atoi (cfg.cfg_raw.sat);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (cfg.cfg_ok.sat != tempInt)
        {
            cfg.cfg_ok.sat = tempInt;
            changeColor = true;
            Serial.println ("Saturation changed");
        }
    }
    else
    {
        itoa (cfg.cfg_ok.sat, cfg.cfg_raw.sat, 10);
    }

    tempInt = atoi (cfg.cfg_raw.bri);
    if (tempInt >= 0 && tempInt < 256)
    {
        if (cfg.cfg_ok.bri != tempInt)
        {
            cfg.cfg_ok.bri = tempInt;
            changeColor = true;
            Serial.println ("Brightness changed");
        }
    }
    else
    {
        itoa (cfg.cfg_ok.bri, cfg.cfg_raw.bri, 10);
    }

    bool tempNtpUse = (0U == strcmp (cfg.cfg_raw.ntpUse, "yes")) && (net.getMode () == Networking::Mode::API);
    if (cfg.cfg_ok.ntpUse != tempNtpUse)
    {
        cfg.cfg_ok.ntpUse = tempNtpUse;
        changeTimeCfg = true;
        Serial.println ("ntpUse changed");
    }

    if (cfg.cfg_ok.ntpUse)
    {
        if ((0U != strcmp(cfg.cfg_raw.ntpServer, ntpServer)) && (0U != strcmp(cfg.cfg_raw.ntpServer, "")))
        {
            // String changed and is not empty
            ntpServer = cfg.cfg_raw.ntpServer;
            changeTimeCfg = true;
            Serial.println ("ntpServer changed");
        }
    }

    if ((changeColor or changeTimeCfg) && !init)
    {
        Serial.println ("Colors or TimeCfg updated");
        zeigeNachrichtOk ();
        countdown2 = 0;
    }

    if (changeTimeCfg or init)
    {
        Serial.print ("Activate NTP");
        WiFi.config (0U, 0U, 0U);
        delay (5000);
        //cm.startAPI();
        configTime (gmtOffset_sec, daylightOffset_sec, "fritz.box");
        setenv ("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);
        getLocalTime (&timeinfo);
        ntpZuletztVerwendet = true;
    }

    if (!cfg.cfg_ok.ntpUse)
    {
        tempInt = atoi (cfg.cfg_raw.hour);
        if (tempInt >= 0 && tempInt < 24)
        {
            if (cfg.cfg_ok.hour != tempInt)
            {
                cfg.cfg_ok.hour = tempInt;
                changeTime = true;
                Serial.println ();
                Serial.print ("hour changed to ");
                Serial.print (cfg.cfg_ok.hour);
            }
        }
        else
        {
            itoa (cfg.cfg_ok.hour, cfg.cfg_raw.hour, 10);
        }

        tempInt = atoi (cfg.cfg_raw.minute);
        if (tempInt >= 0 && tempInt < 24)
        {
            if (cfg.cfg_ok.minute != tempInt)
            {
                cfg.cfg_ok.minute = tempInt;
                changeTime = true;
                Serial.println ();
                Serial.print ("minute changed to ");
                Serial.print (cfg.cfg_ok.minute);
            }
        }
        else
        {
            itoa (cfg.cfg_ok.minute, cfg.cfg_raw.minute, 10);
        }
    }

    if (changeColor or init)
    {
        setzeFarben ();
    }

    if (changeTime)
    {
        if (!init)
        {
            zeigeNachrichtOk ();
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
        long newTime = cfg.cfg_ok.hour * 3600 + cfg.cfg_ok.minute * 60;

        configTime (newTime - currentTime, daylightOffset_sec, ntpServer);
        ntpZuletztVerwendet = false;
    }
}


void Wordclock::checkWifi (bool init)
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
            zeigePasswort ();
            IPAddress local = WiFi.softAPIP ();
            zeigeIPAdresse (local, 0, 3);
        }
        else if (wifiVerbunden)
        {
            cf.reiheWortOnlineInListe (3, WORDLAYOUT);
            cf.bearbeiteListe (4);
            delay (2000);
            cf.bearbeiteListe (6);
            IPAddress local = WiFi.localIP ();
            zeigeIPAdresse (local, 3, 3);
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
