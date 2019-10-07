#ifndef __WORDCLOCK_H__
#define __WORDCLOCK_H__

#include <IPAddress.h>
#include "Observer.hpp"
#include "Networking.hpp"
#include "ClockFace.hpp"
#include "Config.hpp"

class Wordclock : public Observer
{
public:
    Wordclock(Networking& net, ClockFace& cf, Config& cfg);

    /**
     * Implement method from abstract class Observer.
     */
    virtual void notify();

    void loop();
    void setzeFarben ();
    void zeigeNachrichtOk ();
    void zeigePasswort ();
    void zeigeIPAdresse (IPAddress ip, int startOktett, int endeOktett);

    void checkWifi(bool init);
    void checkConfig(bool init);
private:
    // Reference to Networking
    Networking& net;
    // Reference to ClockFace
    ClockFace& cf;
    // Reference to Config
    Config& cfg;

    const char *ntpServer = "fritz.box";

    const long gmtOffset_sec = 3600;
    const int daylightOffset_sec = 3600;

    int lastTimeDisplayed;
    uint8_t currentHour;
    uint8_t currentMinute;

    uint8_t lastMinute;

    bool wifiVerbunden;
    bool inAPMode;
    bool ntpZuletztVerwendet; //relevant für manuelle Zeiteinstellung
};

#endif /* __WORDCLOCK_H__ */
