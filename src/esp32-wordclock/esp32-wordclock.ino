#include <FastLED.h>
#include "QList.h"
#include "ConfigManager.h"
#include "time.h"
#include <WiFi.h>
#include "display.hpp"



const int WORDLAYOUT = 1;


int aktuelleMinute;
int aktuelleStunde;
int letzteMinute;
int letzteStunde;
int lastTimeDisplayed;

struct Config {
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

struct castedConfig {
  boolean ntpUse = true;
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

boolean wifiVerbunden;
boolean inAPMode;
boolean ntpZuletztVerwendet = false; //relevant für manuelle Zeiteinstellung

uint16_t countdown;
uint8_t countdown2;

struct tm timeinfo;

const char mimeHTML[] PROGMEM = "text/html";
const char *configHTMLFile = "/settings.html";

const char* ntpServer = "fritz.box";

const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

String wordsMinute[12] = {"oclock", "five past", "ten past", "fifteen past", "twenty past", "twentyfive past", "half past", "twentyfive to", "twenty to", "fifteen to", "ten to", "five to"};
String wordsHour[12] = {"twelve", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven"};

struct Metadata {
  int8_t version;
} meta;

ConfigManager configManager;

void createCustomRoute(WebServer *server) {
  server->on(
      "/settings.html",
      HTTPMethod::HTTP_GET,
      [server]()
      {
        SPIFFS.begin();
    
        File f = SPIFFS.open(configHTMLFile, "r");
        if (!f) {
          Serial.println(F("file open failed"));
          server->send(404, FPSTR(mimeHTML), F("File not found 5"));
          return;
        }
    
        server->streamFile(f, FPSTR(mimeHTML));
    
        f.close();
      }
    );
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Setup started");

  display_setup();

  meta.version = 3; //ConfigManagaer

  Serial.println("Init ConfigManager");
  configManager.setAPName("Wordclock");
  configManager.setAPFilename("/index.html");

  configManager.addParameter("hour",      config.hour,   10);
  configManager.addParameter("minute",    config.minute, 10);
  configManager.addParameter("ntpUse",    config.ntpUse, 10);
  configManager.addParameter("ntpServer", config.ntpServer, 20);
  //configManager.addParameter("mqtt_password", config.mqtt_password, 10, set);
  configManager.addParameter("c1",        config.c1,     10);
  configManager.addParameter("c2",        config.c2,     10);
  configManager.addParameter("c3",        config.c3,     10);
  configManager.addParameter("c4",        config.c4,     10);
  configManager.addParameter("sat",       config.sat,    10);
  configManager.addParameter("bri",       config.bri,    10);
  configManager.addParameter("enabled",   &config.enabled);
  configManager.addParameter("version",   &meta.version, get);
  configManager.setAPICallback(createCustomRoute);
  configManager.setAPCallback(createCustomRoute);
  configManager.begin(config);

  Serial.println("Init Colors");
  checkConfig(true);
  checkWifi(true);

  letzteStunde = -1;
  letzteMinute = -1;
  lastTimeDisplayed = -1;
  countdown = 0;
  countdown2 = 0;
}

void setzeFarben()
{
  Serial.println();
  Serial.print("Setze Farben: ");
  Serial.print(validConfig.c1);
  Serial.print(", ");
  Serial.print(validConfig.c2);
  Serial.print(", ");
  Serial.print(validConfig.c3);
  Serial.print(", ");
  Serial.print(validConfig.c4);

  reiheWortItsInListe(1);
  setzeFarbe(validConfig.c1);
  for (int i = 1; i < 12; i++)
  {
    reiheMinutenInListe(i, 1, WORDLAYOUT);
  }
  setzeFarbe(validConfig.c2);
  for (int i = 0; i < 12; i++)
  {
    reiheStundenInListe(i, 1, WORDLAYOUT);
  }
  setzeFarbe(validConfig.c4);
  reiheWortPastInListe(1, WORDLAYOUT);
  setzeFarbe(validConfig.c3);
  reiheWortToInListe(1, WORDLAYOUT);
  setzeFarbe(validConfig.c3);
  reiheWortOclockInListe(1, WORDLAYOUT);
  setzeFarbe(validConfig.c1);
}



void loop()
{
  // Always do...
  configManager.loop();
  checkConfig(false);
  checkWifi(false);

  // Periodically
  // a) Flash LEDs every ~ 10*countdown milliseconds
  // b) Update time every ~ 10*countdown*countdown2 milliseconds
  if (countdown > 0)
  {
    countdown--;
  }
  else
  {
    // Countdown1 is zero

    // Reload
    countdown = 1000;
    
    if (countdown2 > 0)
    {
      // Countdown2 is not zero --> flash LEDs
      Serial.println("Flash LEDs");
      countdown2--;
      bearbeiteListe(3);
    }
    else
    {
      // Countdown2 is zero, too --> Update time

      // Reload
      countdown2 = 10;
      
      Serial.println();
      Serial.print("Updating time...");
      getLocalTime(&timeinfo);
      Serial.print("Done");
  
      uint8_t internalHour = timeinfo.tm_hour;
      uint8_t internalMinute = timeinfo.tm_min;
      uint8_t tempHour = internalHour % 12;
      uint8_t tempMinute = (internalMinute + 2) % 60 / 5;
  
      if (internalMinute > 32)
      {
        tempHour = (tempHour + 1) % 12;
      }
  
      Serial.println();
      Serial.print("Current time: ");
      Serial.print(internalHour);
      Serial.print(":");
      Serial.print(internalMinute);
      Serial.print(":");
      Serial.print(timeinfo.tm_sec);
  
      if (tempHour != letzteStunde or tempMinute != letzteMinute)
      {
        Serial.println();
        Serial.print("Stelle Uhrzeit um von Stunde ");
        Serial.print(letzteStunde);
        Serial.print(" auf ");
        Serial.print(tempHour);
        Serial.print(" und Minute ");
        Serial.print(letzteMinute);
        Serial.print(" auf ");
        Serial.print(tempMinute);
        Serial.println();
        Serial.print("In Worten: its ");
        Serial.print(wordsMinute[tempMinute]);
        Serial.print(" (");
        Serial.print(tempMinute);
        Serial.print(") ");
        Serial.print(wordsHour[tempHour]);
        Serial.print(" (");
        Serial.print(tempHour);
        Serial.print(") ");
  
        clearLists();
  
        if (letzteStunde != -1) {
          reiheWortItsInListe(0);  //kein Ausblenden wenn vorher keine valide Zeit angezeigt wurde
        }
        reiheWortItsInListe(1);
        reiheWortItsInListe(2);
        reiheDummiesInListe(3, 2);
  
        if (letzteMinute > 0)
        {
          reiheMinutenInListe(letzteMinute, 0, WORDLAYOUT);
          reihePastOderToInListe(letzteMinute, 0, WORDLAYOUT);
        }
  
        if (tempMinute > 0)
        {
          reiheMinutenInListe(tempMinute, 1, WORDLAYOUT);
          reiheMinutenInListe(tempMinute, 2, WORDLAYOUT);
          reiheDummiesInListe(3, 2);
          reihePastOderToInListe(tempMinute, 1, WORDLAYOUT);
          reihePastOderToInListe(tempMinute, 2, WORDLAYOUT);
          reiheDummiesInListe(3, 2);
        }
  
        if (letzteStunde != -1)
        {
          reiheStundenInListe(letzteStunde, 0, WORDLAYOUT);
        }
  
        if (tempHour != -1)
        {
          reiheStundenInListe(tempHour, 1, WORDLAYOUT);
          reiheStundenInListe(tempHour, 2, WORDLAYOUT);
          reiheDummiesInListe(3, 2);
        }
  
        if (letzteMinute == 0)
        {
          reiheWortOclockInListe(0, WORDLAYOUT);
        }
  
        if (tempMinute == 0)
        {
          reiheWortOclockInListe(1, WORDLAYOUT);
          reiheWortOclockInListe(2, WORDLAYOUT);
        }
  
        bearbeiteListe(2);
        letzteStunde = tempHour;
        letzteMinute = tempMinute;
      }
      else
      {
        Serial.println("Keine Veränderung der Zeitanzeige");
        bearbeiteListe(3);
      }
  
      Serial.println("--------------");
      Serial.println();
    }
  }
  delay(10);
  // Sleep for 5 seconds
//  esp_sleep_enable_timer_wakeup(5e6);
//  esp_light_sleep_start();
//  Serial.println("Wake up.");

}

void checkConfig(boolean init)
{
  boolean changeColor = false;
  boolean changeTimeCfg = false;
  boolean changeTime = false;

  int tempInt = atoi(config.c1);
  if (validConfig.c1 != tempInt and tempInt > 0 and tempInt <= 256)
  {
    validConfig.c1 = tempInt;
    changeColor = true;
    Serial.println("Color 1 changed");
  }

  tempInt = atoi(config.c2);
  if (validConfig.c2 != tempInt and tempInt > 0 and tempInt <= 256)
  {
    validConfig.c2 = tempInt;
    changeColor = true;
    Serial.println("Color 2 changed");
  }

  tempInt = atoi(config.c3);
  if (validConfig.c3 != tempInt and tempInt > 0 and tempInt <= 256)
  {
    validConfig.c3 = tempInt;
    changeColor = true;
    Serial.println("Color 3 changed");
  }

  tempInt = atoi(config.c4);
  if (validConfig.c4 != tempInt and tempInt > 0 and tempInt <= 256)
  {
    validConfig.c4 = tempInt;
    changeColor = true;
    Serial.println("Color 4 changed");
  }

  tempInt = atoi(config.sat);
  if (validConfig.sat != tempInt and tempInt > 0 and tempInt <= 256)
  {
    validConfig.sat = tempInt;
    changeColor = true;
    Serial.println("sat changed");
  }

  tempInt = atoi(config.bri);
  if (validConfig.bri != tempInt and tempInt > 0 and tempInt <= 256)
  {
    validConfig.bri = tempInt;
    changeColor = true;
    Serial.println("bri changed");
  }

  boolean tempNtpUse = strcmp(config.ntpUse, "yes") == 0;
  if (validConfig.ntpUse != tempNtpUse)
  {
    validConfig.ntpUse = tempNtpUse;
    changeTimeCfg = true;
    Serial.println("ntpUse changed");
  }

  if (validConfig.ntpUse)
  {
    if (config.ntpServer != ntpServer and config.ntpServer != "")
    {
      ntpServer = config.ntpServer;
      changeTimeCfg = true;
      Serial.println("ntpServer changed");
    }
  }

  if ((changeColor or changeTimeCfg) and !init)
  {
    Serial.println("Colors or TimeCfg updated");
    zeigeNachrichtOk();
    countdown2 = 0;
  }

  if (changeTimeCfg or init)
  {

    Serial.print("Activate NTP");
    WiFi.config(0U, 0U, 0U);
    FastLED.delay(5000);
    //configManager.startAPI();
    configTime(gmtOffset_sec, daylightOffset_sec, "fritz.box");
    setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);
    getLocalTime(&timeinfo);
    ntpZuletztVerwendet = true;

  }

  if (!validConfig.ntpUse)
  {
    tempInt = atoi(config.hour);
    if (validConfig.hour != tempInt and tempInt >= 0 and tempInt < 24)
    {
      validConfig.hour = tempInt;
      changeTime = true;
      Serial.println();
      Serial.print("hour changed to ");
      Serial.print(validConfig.hour);
    }

    tempInt = atoi(config.minute);
    if (validConfig.minute != tempInt and tempInt >= 0 and tempInt < 60)
    {
      validConfig.minute = tempInt;
      changeTime = true;
      Serial.println();
      Serial.print("minute changed to ");
      Serial.print(validConfig.minute);
    }
  }

  if (changeColor or init)
  {
    setzeFarben();
  }

  if (changeTime)
  {
    if (!init)
    {
      zeigeNachrichtOk();
      countdown2 = 0;
    }
    if (!ntpZuletztVerwendet)
    {
      configTime(0, daylightOffset_sec, ntpServer);
    }
    getLocalTime(&timeinfo);
    long currentTime = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60;
    if (ntpZuletztVerwendet)
    {
      currentTime -= daylightOffset_sec;
    }
    long newTime = validConfig.hour * 3600 + validConfig.minute * 60;

    configTime(newTime - currentTime, daylightOffset_sec, ntpServer);
    ntpZuletztVerwendet = false;
  }
}

void zeigeNachrichtOk()
{
  bearbeiteListe(7);
  int offsetLayout = 0;
  if (WORDLAYOUT == 2)
  {
    offsetLayout = 2;
  }

  int wortOk[2][2] =
  {
    5 - offsetLayout, 7,
    7 - offsetLayout, 7
  };
  reiheLedsInListe(wortOk, 2, 3, WORDLAYOUT);
  bearbeiteListe(4);
  FastLED.delay(2000);
  bearbeiteListe(6);
  bearbeiteListe(8);
}

void checkWifi(boolean init)
{
  boolean inAPModeAktuell = configManager.getMode() == 0;
  boolean wifiAktuell = WiFi.status() == WL_CONNECTED;
  if (init or wifiAktuell != wifiVerbunden or inAPModeAktuell != inAPMode)
  {
    wifiVerbunden = wifiAktuell;
    inAPMode = inAPModeAktuell;
    bearbeiteListe(7);
    if (inAPMode)
    {
      reiheWortWifiInListe(3, WORDLAYOUT);
      bearbeiteListe(4);
      FastLED.delay(2000);
      bearbeiteListe(6);
      IPAddress local = WiFi.softAPIP();
      zeigeIPAdresse(local, 0, 3);
    }
    else if (wifiVerbunden)
    {
      reiheWortOnlineInListe(3, WORDLAYOUT);
      bearbeiteListe(4);
      FastLED.delay(2000);
      bearbeiteListe(6);
      IPAddress local = WiFi.localIP();
      zeigeIPAdresse(local, 3, 3);
    }
    else if (!wifiVerbunden)
    {
      reiheWortOfflineInListe(3, WORDLAYOUT);
      bearbeiteListe(5);
      FastLED.delay(2000);
      bearbeiteListe(6);
    }
    bearbeiteListe(8);
  }
}

void zeigeIPAdresse(IPAddress ip, int startOktett, int endeOktett)
{
  Serial.println();
  Serial.print("Zeige IP Adresse auf Uhr: ");
  Serial.print(ip);
  for (int i = startOktett; i <= endeOktett; i++)
  {
    Serial.println();
    uint8_t oktettInt = ip[i];
    Serial.print(oktettInt);
    char oktettChar[3];
    itoa(oktettInt, oktettChar, 10);
    for (int j = 0; j < 3; j++)
    {
      Serial.print(" -> ");
      int ziffer = oktettChar[j] - '0';
      Serial.print(ziffer);
      if (ziffer > 0 and ziffer <= 9)
      {
        reiheStundenInListe(ziffer, 3, WORDLAYOUT);
        bearbeiteListe(4);
        FastLED.delay(500);
        bearbeiteListe(6);
      }
      else if (ziffer == 0)
      {
        reiheStundenInListe(10, 3, WORDLAYOUT);
        bearbeiteListe(4);
        FastLED.delay(500);
        bearbeiteListe(6);
      }
      else
      {
        break;  //Ende der Zahl
      }
    }
  }
}
