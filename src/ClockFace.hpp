#ifndef __CLOCKFACE_H__
#define __CLOCKFACE_H__

#include "QList.h"

class ClockFace
{
public:
    ClockFace() {};
    void display_setup(void);
    void bearbeiteListe(int modus);
    void clearLists(int liste);
    void iteriereLedsInListe(int liste, int aktion, bool onebyone, int delay);
    void reiheStundenInListe(int stunde, int liste, int wordLayout);
    void reiheStundenInListeLayout1(int stunde, int liste, int wordlayout);
    void reiheStundenInListeLayout2(int stunde, int liste, int wordlayout);
    void reiheStundenInListeLayout3(int stunde, int liste, int wordlayout);
    void reiheWortPastInListe(int liste, int wordLayout);
    void reiheWortToInListe(int liste, int wordLayout);
    void reiheWortOclockInListe(int liste,int wordLayout);
    void reiheMinutenInListe(int nMalFuenf, int liste, int wordLayout);
    void reihePastOderToInListe(int nMalFuenf, int liste, int wordLayout);
    void reiheWortOnlineInListe(int liste, int wordLayout);
    void reiheWortOfflineInListe(int liste, int wordLayout);
    void reiheWortWifiInListe(int liste, int wordLayout);
    void reiheWortItsInListe(int liste);
    void reiheLedsInListe(int wort[][2], int laenge, int liste, int wordLayout);

    void setzeFarbe(uint16_t farbeLeds);
    void reiheDummiesInListe(int anzahl, int liste);
private:

    void createMatrix(int variante);

    void turnMatrix(int orientation);



    void reiheLedInListe(int led, int liste);

    int berechneHelligkeit(int schritt, int ledIndex, int schrittweite, int offset, int volleHelligkeit, bool aufbau);
    void fadeLeds();
    void flashLeds();

};

#endif /* __CLOCKFACE_H__ */

