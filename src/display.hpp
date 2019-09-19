
#include "QList.h"

void display_setup(void);
void clearLists(int liste);

void createMatrix(int variante);

void turnMatrix(int orientation);


void reiheWortItsInListe(int liste);

void reiheLedsInListe(int wort[][2], int laenge, int liste, int wordLayout);

void reiheLedInListe(int led, int liste);

void reiheDummiesInListe(int anzahl, int liste);

void bearbeiteListe(int modus);

void iteriereLedsInListe(int liste, int aktion, boolean onebyone, int delay);


void fadeLeds();

int berechneHelligkeit(int schritt, int ledIndex, int schrittweite, int offset, int volleHelligkeit, boolean aufbau);


void flashLeds();


void setzeFarbe(uint16_t farbeLeds);

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
