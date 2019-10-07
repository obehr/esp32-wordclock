#ifndef __CLOCKFACE_H__
#define __CLOCKFACE_H__

#include <FastLED.h>
#include "QList.h"

class ClockFace
{
public:
    ClockFace(int MATRIXLAYOUT, int ORIENTATION);
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
    static const int DATA_PIN;
    int MATRIXLAYOUT;
    int ORIENTATION;

    CRGBArray<64> matrix;

    QList<int> listeAufbau;
    QList<int> listeAbbau;
    QList<int> listeEffekt;
    QList<int> listeNachricht;

    uint8_t matrixLeds[8][8];
    uint16_t ledFarbzuweisung[64];

    static const char matrixZiffernblattLayout1[8][8];
    static const char matrixZiffernblattLayout2[8][8];
    static const char matrixZiffernblattLayout3[8][8];

    void createMatrix(int variante);
    void turnMatrix(int orientation);
    void reiheLedInListe(int led, int liste);
    int berechneHelligkeit(int schritt, int ledIndex, int schrittweite, int offset, int volleHelligkeit, bool aufbau);
    void fadeLeds();
    void flashLeds();

};

#endif /* __CLOCKFACE_H__ */

