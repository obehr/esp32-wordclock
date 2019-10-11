#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#include <DNSServer.h>

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
    #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32) //ESP32
    #include <WiFi.h>
#endif

class Networking {
public:
    Networking(DNSServer& dnsServer);
    void setAPName(const char *name);
    void setAPPassword(const char *password);
    void setAPTimeout(const int timeout);
    void setWifiConnectRetries(const int retries);
    void setWifiConnectInterval(const int interval);
    enum Mode {AP, API};
    Mode getMode();
    void loop();

    bool wifiConnect();
    int apTimeout = 0;
    void startAP();
        void startApi();
private:
    Mode mode;

    char *apName = (char *)"Thing";
    char *apPassword = NULL;
    unsigned long apStart = 0;

    int wifiConnectRetries = 20;
    int wifiConnectInterval = 500;

    DNSServer& dnsServer;







};

#endif /* __NETWORKING_H__ */

