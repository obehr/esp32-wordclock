
#include <HardwareSerial.h>
#include <FastLED.h>
#include "Wordclock.hpp"
#include "ClockFace.hpp"
#include "time.h"

extern tm timeinfo;

String wordsMinute[12] =
    { "oclock", "five past", "ten past", "fifteen past", "twenty past", "twentyfive past", "half past", "twentyfive to",
                    "twenty to", "fifteen to", "ten to", "five to" };
String wordsHour[12] =
    { "twelve", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven" };

int letzteMinute;
int letzteStunde;

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
};

extern castedConfig validConfig;
const int WORDLAYOUT = 1;

uint16_t countdown;
uint8_t countdown2;

Wordclock::Wordclock(ClockFace& cf) : cf(cf)
{
    letzteStunde = -1;
        letzteMinute = -1;
    countdown = 0;
    countdown2 = 0;
}

void Wordclock::loop()
{
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
    Serial.print (validConfig.c1);
    Serial.print (", ");
    Serial.print (validConfig.c2);
    Serial.print (", ");
    Serial.print (validConfig.c3);
    Serial.print (", ");
    Serial.print (validConfig.c4);

    cf.reiheWortItsInListe (1);
    cf.setzeFarbe (validConfig.c1);
    for (int i = 1; i < 12; i++)
    {
        cf.reiheMinutenInListe (i, 1, WORDLAYOUT);
    }
    cf.setzeFarbe (validConfig.c2);
    for (int i = 0; i < 12; i++)
    {
        cf.reiheStundenInListe (i, 1, WORDLAYOUT);
    }
    cf.setzeFarbe (validConfig.c4);
    cf.reiheWortPastInListe (1, WORDLAYOUT);
    cf.setzeFarbe (validConfig.c3);
    cf.reiheWortToInListe (1, WORDLAYOUT);
    cf.setzeFarbe (validConfig.c3);
    cf.reiheWortOclockInListe (1, WORDLAYOUT);
    cf.setzeFarbe (validConfig.c1);
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
    FastLED.delay (2000);
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
    FastLED.delay (1000);
    cf.clearLists (3);

    int wortSechsteZeile[10][2] =
        { 0, 5, 1, 5, 2, 5, 3, 5, 4, 5, 5, 5, 6, 5, 7, 5 };
    cf.reiheLedsInListe (wortSechsteZeile, 8, 3, WORDLAYOUT);
    cf.bearbeiteListe (4);
    FastLED.delay (10000);
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
    FastLED.delay (2000);
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
                FastLED.delay (500);
                cf.bearbeiteListe (6);
            }
            else if (ziffer == 0)
            {
                cf.reiheStundenInListe (10, 3, WORDLAYOUT);
                cf.bearbeiteListe (4);
                FastLED.delay (500);
                cf.bearbeiteListe (6);
            }
            else
            {
                break;  //Ende der Zahl
            }
        }
    }
}
