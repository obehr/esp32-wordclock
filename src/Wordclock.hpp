#ifndef __WORDCLOCK_H__
#define __WORDCLOCK_H__

#include <IPAddress.h>
#include "ClockFace.hpp"

class Wordclock
{
public:
    Wordclock(ClockFace& cf);
    void loop();
    void setzeFarben ();
    void zeigeNachrichtOk ();
    void zeigePasswort ();
    void zeigeIPAdresse (IPAddress ip, int startOktett, int endeOktett);
private:
    // Reference to ClockFace
    ClockFace& cf;
};

#endif /* __WORDCLOCK_H__ */
