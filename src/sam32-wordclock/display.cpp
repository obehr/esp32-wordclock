#include <FastLED.h>
#include "QList.h"

#include "display.hpp"


const int DATA_PIN = 13;
const int MATRIXLAYOUT = 1;
const int ORIENTATION = 1;

const char matrixZiffernblattLayout1[8][8] = 
{
  'i', 't', 's', 'o', 'f', 't', 'w', 'e',
  'n', 't', 'y', 'f', 'i', 'v', 'e', 'n',
  'h', 'a', 'l', 'f', 'p', 'a', 's', 't',
  't', 'o', 's', 't', 'w', 'o', 'n', 'e',
  'e', 'i', 'g', 'h', 'e', 'l', 'i', 'n',
  's', 'o', 'u', 'r', 's', 'e', 'v', 'e',
  'i', 'f', 'i', 'v', 'e', 't', 'e', 'n',
  'x', 'n', 'o', 'c', 'l', 'o', 'c', 'k'
};


const char matrixZiffernblattLayout2[8][8] = 
{
  'i', 't', 's', 'o', 'f', 't', 'w', 'e',
  'n', 't', 'y', 'f', 'i', 'v', 'e', 'n',
  'h', 'a', 'l', 'f', 'p', 'a', 's', 'o',
  'n', 'f', 'x', 't', 'w', 'e', 't', 's',
  'e', 'i', 'g', 'h', 't', 'o', 'l', 'e',
  's', 'v', 'n', 'e', 'h', 't', 'n', 'v',
  'w', 'f', 'f', 'o', 'u', 'r', 'e', 'e',
  'o', 'c', 'l', 'o', 'c', 'k', 'f', 'n'
};

const char matrixZiffernblattLayout3[8][8] = 
{
  'i', 't', 's', 'o', 'f', 't', 'w', 'e',
  'n', 't', 'y', 'f', 'i', 'v', 'e', 'n',
  'h', 'a', 'l', 'f', 'p', 'a', 's', 't',
  't', 'o', 'e', 'f', 't', 'w', 'e', 's',
  'f', 'n', 'i', 'g', 'h', 'o', 'l', 'e',
  'o', 's', 'v', 'n', 'r', 'n', 'e', 'v',
  'u', 'i', 'x', 'e', 't', 'e', 'n', 'e',
  'r', 'n', 'o', 'c', 'l', 'o', 'c', 'k'
};

CRGBArray<64> matrix;

QList<int> listeAufbau;
QList<int> listeAbbau;
QList<int> listeEffekt;
QList<int> listeNachricht;

uint8_t matrixLeds[8][8];
uint16_t ledFarbzuweisung[64];


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
};

// Use global variable. It is defined somewhere else (in a .cpp file).
extern castedConfig validConfig;

void display_setup(void)
{
  Serial.println("Init LED Matrix");
  createMatrix(MATRIXLAYOUT);
  turnMatrix(ORIENTATION);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(matrix, 64);
}

void clearLists(void)
{
  listeAufbau.clear();
  listeAbbau.clear();
  listeEffekt.clear();
}

void createMatrix(int variante)
{
  int x=8;
  int y=8;
  if(variante==1) 
  /*
   * 0  1  2  3  4  5  6  7
   * 8  9  10 11 12 13 14 15
   * 16 17 18 19 20 21 22 23
   * ...
   */
  {
    for(int yi=0; yi<y; yi++)
    {
     for(int xi=0; xi<x; xi++)
      {
        matrixLeds[xi][yi]= xi + yi*x;
      }
    }
  }
  else if(variante==2) 
  /*
   * 0  1  2  3  4  5  6  7
   * 15 14 13 12 11 10 9  8
   * 16 17 18 19 20 21 22 23
   * ...
   */
  {
    for(int yi=0; yi<y; yi++)
    {
      for(int xi=0; xi<x; xi++)
      {
        matrixLeds[xi][yi]= xi + yi*x;
      }
      yi++;
      for(int xi=0; xi<x; xi++)
      {
        matrixLeds[7-xi][yi]= xi + yi*x;
      }
    }
  }
}

void turnMatrix(int orientation)
{
  if(orientation>0 and orientation <4)
  {
    Serial.println("Drehe Matrix. Ergebnis:");
    int matrixTemp[8][8];
    int x=8;
    int y=8;
    for(int yi=0; yi<y; yi++)
    {
      for(int xi=0; xi<x; xi++)
      {
        if(orientation==1) //turn once clockwise 90degree
        { matrixTemp[xi][yi] = matrixLeds[x-yi-1][xi]; }
        
        else if(orientation==2) //turn twice 90degree
        { matrixTemp[xi][yi] = matrixLeds[x-xi-1][y-yi-1]; }
        
        else if(orientation==3) //turn once counter clockwise 90degree
        { matrixTemp[xi][yi] = matrixLeds[yi][y-xi-1]; }
      }
    }
    for(int yi=0; yi<y; yi++)
    {
      Serial.println();
      for(int xi=0; xi<x; xi++)
      { 
        matrixLeds[xi][yi] = matrixTemp[xi][yi]; 
        Serial.print(matrixLeds[xi][yi]);
        Serial.print(" ");
      }
    }
  }
}


void reiheWortItsInListe(int liste)
{
  Serial.println();
  Serial.print("->Wort ITS hinzufuegen: ");
  int wortIts[3][2] = 
  {
    0,0,
    1,0,
    2,0    
  };
  reiheLedsInListe(wortIts, 3, liste, 1);
}

void reiheLedsInListe(int wort[][2], int laenge, int liste, int wordLayout)
{
  Serial.println();
  Serial.print("Leds in Liste ");
  Serial.print(liste);
  Serial.print(" einreihen: ");
  
  int x;
  int y;
  uint8_t ledId;
  for(int i=0; i<laenge; i++)
  {
    x=wort[i][0];
    y=wort[i][1];
    ledId=matrixLeds[x][y];
    Serial.print(ledId);
    reiheLedInListe(ledId, liste);
    Serial.print("(");
    if(wordLayout==1)
    { Serial.print(matrixZiffernblattLayout1[y][x]); }
    else if(wordLayout==2)
    { Serial.print(matrixZiffernblattLayout2[y][x]); }
    else if(wordLayout==3)
    { Serial.print(matrixZiffernblattLayout3[y][x]); }
    Serial.print(") ");
  }
  Serial.println();
  Serial.print("--------------");
}

void reiheLedInListe(int led, int liste)
{
  
  if(liste==0)
  {
    if(listeAbbau.indexOf(led)==-1)
    { listeAbbau.push_back(led); }
  }
  else if(liste==1)
  {
    if(listeAufbau.indexOf(led)==-1)
    { listeAufbau.push_back(led); }
  }
  else if(liste==2)
  {
    listeEffekt.push_back(led);
  }
  else if(liste==3)
  {
    listeNachricht.push_back(led);
  }  
}

void reiheDummiesInListe(int anzahl, int liste)
{
  for(int i=1;i<=anzahl; i++)
  { reiheLedInListe(-1,liste); }
}

void bearbeiteListe(int modus)
{
  if(modus==1) //schalten
  { 
    iteriereLedsInListe(0,0,true,100);
    iteriereLedsInListe(1,1,true,100);
    listeAufbau.clear();
    listeAbbau.clear();
  }
  else if(modus==2) //fade over
  {
    fadeLeds();
    listeAufbau.clear();
    listeAbbau.clear();
  }
  else if(modus==3) //short flash
  {
    flashLeds();
  }
  else if(modus==4) //notify green
  {
    iteriereLedsInListe(3, 3, true, 100);
//    zeigeNachricht(96);
  }
  else if(modus==5) //notify red
  {
    iteriereLedsInListe(3, 4, true, 100);
//    zeigeNachricht(0);
  }
  else if(modus==6) //notify off
  {
    iteriereLedsInListe(3, 0, true, 100);
    listeNachricht.clear();
  }
  else if(modus==7) //before notify
  {
    iteriereLedsInListe(2,2,false,100);
  }
  else if(modus==8) //after notify
  {
    iteriereLedsInListe(1,1,false,100);
  }
}

void iteriereLedsInListe(int liste, int aktion, boolean onebyone, int delay)
{
  /*
  Aktion 0: ausschalten
  Aktion 1: einschalten
  Aktion 2: FadeToBlackBy128
  Aktion 3: grün färben
  Aktion 4: rot färben
  */

  Serial.println();
  Serial.print("Iteriere Liste ");
  Serial.print(liste);
  Serial.print(" für ");
  Serial.print((aktion==0)?"Aktion 0: ausschalten":(aktion==1)?"Aktion 1: einschalten":(aktion==2)?"Aktion 2: FadeToBlackBy128":(aktion==3)?"Aktion 3: grün färben":(aktion==4)?"Aktion 3: grün färben":"ungueltige Aktion");
  Serial.print((onebyone)?" onebyone Delay ":" Delay pro Durchgang ");
  Serial.print(delay);

  int ledId;
  int laengeListe = (liste==0)?listeAbbau.length():(liste==1)?listeAufbau.length():(liste==2)?listeEffekt.length():(liste==3)?listeNachricht.length():-1;

  for(int i=0; i<laengeListe; i++)
  {
    ledId = (liste==0)?listeAbbau[i]:(liste==1)?listeAufbau[i]:(liste==2)?listeEffekt[i]:(liste==3)?listeNachricht[i]:-1;
    
    if(aktion==0) { matrix[ledId].setHSV(0,0,0); }
    else if(aktion==1) { matrix[ledId].setHSV(ledFarbzuweisung[ledId], validConfig.sat, validConfig.bri); }
    else if(aktion==2) { matrix[ledId].fadeToBlackBy(128); }
    else if(aktion==3) { matrix[ledId].setHSV(96, validConfig.sat, validConfig.bri); }
    else if(aktion==4) { matrix[ledId].setHSV(0, validConfig.sat, validConfig.bri); }
    
    if(onebyone) { FastLED.delay(delay); }
  }
  if(!onebyone) { FastLED.delay(delay); }
}


void fadeLeds()
{
/*
  effektdauert gibt die Anzahl der Schritte an
  delay zwischen den Schritten fix
  fade out und fade in passieren gleichzeitig
  wenn eine einzuschaltende led bereits eingeschaltet 
  ist, so wird solange heruntergedimmt wie die Helligkeit
  noch groesser dem aktuellen Zielwert ist
*/
    int schrittweiteAufbau = 25;
    int schrittweiteAbbau = 5;
    int aktuelleHelligkeit;
    int neueHelligkeit;
    int volleHelligkeit = validConfig.bri;
    int ledId;
    int offsetAufbau=50;
    int offsetAbbau=10;
    int ledIndexAbbau;
    int i=0;
    boolean ausstehenderAufbau = true;
    boolean ausstehenderAbbau = true;
    
    while(ausstehenderAufbau or ausstehenderAbbau)
    {
      ausstehenderAufbau = false;
      for(int j=0; j<listeAufbau.length(); j++)
      {     
        ledId = (int)listeAufbau.at(j);
        neueHelligkeit = berechneHelligkeit(i,j,schrittweiteAufbau,offsetAufbau,volleHelligkeit,true);
        if(neueHelligkeit<volleHelligkeit)
        { ausstehenderAufbau = true; }
        ledIndexAbbau = listeAbbau.indexOf(ledId);
        if(ledIndexAbbau==-1)
        { matrix[ledId].setHSV( ledFarbzuweisung[ledId], validConfig.sat, neueHelligkeit); }
        else {
          int helligkeitAbbau = berechneHelligkeit(i,ledIndexAbbau,schrittweiteAbbau,offsetAbbau,volleHelligkeit,false);
          if(neueHelligkeit>helligkeitAbbau)
          {
            listeAbbau.at(ledIndexAbbau)=-1;
            matrix[ledId].setHSV( ledFarbzuweisung[ledId], validConfig.sat, neueHelligkeit);
          }
        }
      }
      
      ausstehenderAbbau = false;
      for(int k=0; k<listeAbbau.length(); k++)
      {
        ledId = (int)listeAbbau.at(k);
        if(ledId>0)
        {
          neueHelligkeit = berechneHelligkeit(i,k,schrittweiteAbbau,offsetAbbau,volleHelligkeit,false);
          if(neueHelligkeit>0)
          { ausstehenderAbbau = true; }
          matrix[ledId].setHSV(ledFarbzuweisung[ledId], validConfig.sat, neueHelligkeit);
        }
      }
      FastLED.delay(50);
      i++;
    }  
}

int berechneHelligkeit(int schritt, int ledIndex, int schrittweite, int offset, int volleHelligkeit, boolean aufbau)
{
  int neueHelligkeit = (aufbau)? schritt*schrittweite-ledIndex*offset : volleHelligkeit-schritt*schrittweite+ledIndex*offset;
  return (neueHelligkeit<0)?0:(neueHelligkeit>volleHelligkeit)?volleHelligkeit:neueHelligkeit;
}


void flashLeds()
{
    //effektdauert gibt die Anzahl der Schritte an
    //delay zwischen den Schritten fix
    //fade out und fade in passieren gleichzeitig
    //wenn eine einzuschaltende led bereits eingeschaltet ist, so wird solange heruntergedimmt wie die Helligkeit noch groesser dem aktuellen Zielwert ist
    
    int normaleHelligkeit = validConfig.bri;
    int maximaleHelligkeit = normaleHelligkeit*1.5;
    maximaleHelligkeit = (maximaleHelligkeit>255)?255:maximaleHelligkeit;
    int differenz = maximaleHelligkeit - normaleHelligkeit;
    int neueHelligkeit;
    int led;
    int ledId;
    int stepsFadeUp=5;
    int stepsFadeDown=25;
    int stepsDelay=3;
    int runde;
    int startPoint;
    int breakPoint;
    int endPoint;
    int anzahlLeds = listeEffekt.length();
    int anzahlRunden = (anzahlLeds-1) * stepsDelay + stepsFadeUp + stepsFadeDown + 1;
    
    for(int i=0; i<anzahlRunden; i++)
    {
      for(int j=0; j<anzahlLeds; j++)
      {
        startPoint = j*stepsDelay;
        breakPoint = startPoint + stepsFadeUp;
        endPoint = breakPoint + stepsFadeDown;
        if(i>startPoint and i<=breakPoint)
        { neueHelligkeit=normaleHelligkeit+(i-startPoint)*(differenz/stepsFadeUp); }
        else if(i>breakPoint and i<=endPoint)
        { neueHelligkeit=maximaleHelligkeit-(i-breakPoint)*(differenz/stepsFadeDown); }
        else
        { 
          neueHelligkeit=-1; 
        }

        if(neueHelligkeit!=-1)
        {
          ledId = (int)listeEffekt.at(j);
          if(ledId!=-1)
          { matrix[ledId].setHSV( ledFarbzuweisung[ledId], validConfig.sat, neueHelligkeit); }
        }
      }
      FastLED.delay(50);
    }      
}


void setzeFarbe(uint16_t farbeLeds)
{
  Serial.println();
  Serial.print("->Setze Farbe: ");
  Serial.print(farbeLeds);
  int ledId;
  for(int i=0; i<listeAufbau.length(); i++)
  { 
    ledId=(int)listeAufbau.at(i);
    ledFarbzuweisung[ledId] = farbeLeds; }
  listeAufbau.clear();
}

void reiheStundenInListe(int stunde, int liste, int wordLayout)
{
   if(stunde>=0 and stunde<12)
  {
    if(wordLayout==1)
    { reiheStundenInListeLayout1(stunde, liste, wordLayout); }
    else if(wordLayout==2)
    { reiheStundenInListeLayout2(stunde, liste, wordLayout); }
    else if(wordLayout==3)
    { reiheStundenInListeLayout3(stunde, liste, wordLayout); }
  }
}

void reiheStundenInListeLayout1(int stunde, int liste, int wordlayout) 
{
  Serial.println();
  Serial.print("->Stunde ");
  Serial.print(stunde);
  Serial.print(" Stunde in Liste: ");
  Serial.print(liste);
  if(stunde==1)
  {
    int wortStunde[3][2] = 
    {
      5,3,
      6,3,
      7,3
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==2)
  {
    int wortStunde[3][2] = 
    {
      3,3,
      4,3,
      5,3
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==3)
  {
    int wortStunde[5][2] = 
    {
      3,3,
      3,4,
      3,5,
      4,6,
      5,5
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==4)
  {
    int wortStunde[4][2] = 
    {
      1,6,
      1,5,
      2,5,
      3,5
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==5)
  {
    int wortStunde[4][2] = 
    {
      1,6,
      2,6,
      3,6,
      4,6
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==6)
  {
    int wortStunde[3][2] = 
    {
      0,5,
      0,6,
      0,7
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==7)
  {
    int wortStunde[5][2] = 
    {
      4,5,
      5,5,
      6,5,
      7,5,
      7,6
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==8)
  {
    int wortStunde[5][2] = 
    {
      0,4,
      1,4,
      2,4,
      3,4,
      3,3
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==9)
  {
    int wortStunde[4][2] = 
    {
      6,3,
      6,4,
      7,4,
      7,5
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==10)
  {
    int wortStunde[3][2] = 
    {
      5,6,
      6,6,
      7,6
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==11)
  {
    int wortStunde[6][2] = 
    {
      4,4,
      5,4,
      5,5,
      6,5,
      6,6,
      7,6
    };
    reiheLedsInListe(wortStunde, 6, liste, wordlayout);
  }
  else if(stunde==0)
  {
    int wortStunde[6][2] = 
    {
      3,3,
      4,3,
      4,4,
      5,4,
      6,5,
      7,5
    };
    reiheLedsInListe(wortStunde, 6, liste, wordlayout);
  }
}

void reiheStundenInListeLayout2(int stunde, int liste, int wordlayout) 
{

  /*
  I T S O F T W E
  N T Y F I V E N
  H A L F P A S O
  N F X T W E T S
  E I G H T O L E
  S V N E H T N V
  W E F O U R E E
  O C L O C K F N
  */
  Serial.println();
  Serial.print("->Stunde ");
  Serial.print(stunde);
  Serial.print(" in Liste: ");
  Serial.print(liste);
  if(stunde==1)
  {
    int wortStunde[3][2] = 
    {
      5,4,
      6,5,
      7,6
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==2)
  {
    int wortStunde[3][2] = 
    {
      3,3,
      4,3,
      5,4
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==3)
  {
    int wortStunde[5][2] = 
    {
      4,4,
      4,5,
      5,6,
      6,6,
      7,6
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==4)
  {
    int wortStunde[4][2] = 
    {
      2,6,
      3,6,
      4,6,
      5,6
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==5)
  {
    int wortStunde[4][2] = 
    {
      1,3,
      1,4,
      1,5,
      1,6
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==6)
  {
    int wortStunde[3][2] = 
    {
      0,5,
      1,4,
      2,3
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==7)
  {
    int wortStunde[5][2] = 
    {
      7,3,
      7,4,
      7,5,
      7,6,
      7,7
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==8)
  {
    int wortStunde[5][2] = 
    {
      0,4,
      1,4,
      2,4,
      3,4,
      4,4
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==9)
  {
    int wortStunde[4][2] = 
    {
      0,3,
      1,4,
      2,5,
      3,5
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==10)
  {
    int wortStunde[3][2] = 
    {
      5,5,
      6,6,
      7,7
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==11)
  {
    int wortStunde[6][2] = 
    {
      5,3,
      6,4,
      7,4,
      7,5,
      7,6,
      7,7
    };
    reiheLedsInListe(wortStunde, 6, liste, wordlayout);
  }
  else if(stunde==0)
  {
    int wortStunde[6][2] = 
    {
      3,3,
      4,3,
      5,3,
      6,4,
      7,5,
      7,6
    };
    reiheLedsInListe(wortStunde, 6, liste, wordlayout);
  }
}

void reiheStundenInListeLayout3(int stunde, int liste, int wordlayout) 
{
  /*
  I T S O F T W E
  N T Y F I V E N
  H A L F P A S T
  T O E F T W E S
  F N I G H O L E
  O S V N R N E V
  U I X E T E N E
  R N O C L O C K
  */
  
  Serial.println();
  Serial.print("->Stunde ");
  Serial.print(stunde);
  Serial.print(" in Liste: ");
  Serial.print(liste);
  if(stunde==1)
  {
    int wortStunde[3][2] = 
    {
      5,4,
      5,5,
      5,6
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==2)
  {
    int wortStunde[3][2] = 
    {
      4,3,
      5,3,
      5,4
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==3)
  {
    int wortStunde[5][2] = 
    {
      4,3,
      4,4,
      4,5,
      5,6,
      6,5
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==4)
  {
    int wortStunde[4][2] = 
    {
      0,4,
      0,5,
      0,6,
      0,7
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==5)
  {
    int wortStunde[4][2] = 
    {
      3,3,
      2,4,
      2,5,
      3,6
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==6)
  {
    int wortStunde[3][2] = 
    {
      1,5,
      1,6,
      2,6
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==7)
  {
    int wortStunde[5][2] = 
    {
      7,3,
      7,4,
      7,5,
      7,6,
      6,6
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==8)
  {
    int wortStunde[5][2] = 
    {
      2,3,
      2,4,
      3,4,
      4,4,
      4,3
    };
    reiheLedsInListe(wortStunde, 5, liste, wordlayout);
  }
  else if(stunde==9)
  {
    int wortStunde[4][2] = 
    {
      1,4,
      2,4,
      3,5,
      3,6
    };
    reiheLedsInListe(wortStunde, 4, liste, wordlayout);
  }
  else if(stunde==10)
  {
    int wortStunde[3][2] = 
    {
      4,6,
      5,6,
      6,6
    };
    reiheLedsInListe(wortStunde, 3, liste, wordlayout);
  }
  else if(stunde==11)
  {
    int wortStunde[6][2] = 
    {
      6,3,
      6,4,
      7,4,
      7,5,
      7,6,
      6,6
    };
    reiheLedsInListe(wortStunde, 6, liste, wordlayout);
  }
  else if(stunde==0)
  {
    int wortStunde[6][2] = 
    {
      4,3,
      5,3,
      6,3,
      6,4,
      7,5,
      7,6
    };
    reiheLedsInListe(wortStunde, 6, liste, wordlayout);
  }
}

void reiheWortPastInListe(int liste, int wordLayout)
{
  Serial.println();
  Serial.print("->Wort Past in Liste ");
  Serial.print(liste);
  int offsetLayout=0;
  if(wordLayout==2)
  { offsetLayout=1; }

  int wortPast[4][2] = 
  {
    4,2,
    5,2,
    6,2,
    7-offsetLayout,2+offsetLayout
  };
  reiheLedsInListe(wortPast, 4, liste, wordLayout);      
}

void reiheWortToInListe(int liste, int wordLayout)
{  
  Serial.println();
  Serial.print("->Wort To in Liste ");
  Serial.print(liste);
  if(wordLayout==2)
  {
    int wortTo[2][2] = 
    {
      6,3,
      7,2
    }; 
    reiheLedsInListe(wortTo, 2, liste, wordLayout);
  }
  else
  {
    int wortTo[2][2] = 
    {
      0,3,
      1,3
    }; 
    reiheLedsInListe(wortTo, 2, liste, wordLayout);
  }
}

void reiheWortOclockInListe(int liste,int wordLayout)
{
  Serial.println();
  Serial.print("->Wort OClock in Liste ");
  Serial.print(liste);
  int offsetLayout=0;
  if(wordLayout==2)
  { offsetLayout=2; }

  int wortOclock[6][2] = 
  {
    2-offsetLayout,7,
    3-offsetLayout,7,
    4-offsetLayout,7,
    5-offsetLayout,7,
    6-offsetLayout,7,
    7-offsetLayout,7
  };
  reiheLedsInListe(wortOclock, 6, liste, wordLayout);
}  


void reiheMinutenInListe(int nMalFuenf, int liste, int wordLayout) 
{
  //0=0, 1=5, 2=10, 3=15, 4=20, 5=25, 6=30, 7=35, 8=40, 9=45, 10=50, 11=55
  Serial.println();
  Serial.print("Minute ");
  Serial.print(nMalFuenf);
  Serial.print(" in Liste: ");
  Serial.print(liste);

    //„twenty“
    //20, 25, 35, 40
  if(nMalFuenf==4 or nMalFuenf==5 or nMalFuenf==7 or nMalFuenf==8)
  {
    int wortMinute[6][2] = 
    {
      5,0,
      6,0,
      7,0,
      0,1,
      1,1,
      2,1
    };
    reiheLedsInListe(wortMinute, 6, liste, wordLayout);
  }
  //„five“
  //5, 25, 35, 55
  if(nMalFuenf==1 or nMalFuenf==5 or nMalFuenf==7 or nMalFuenf==11)
  {
    int wortMinute[4][2] = 
    {
      3,1,
      4,1,
      5,1,
      6,1
    };
    reiheLedsInListe(wortMinute, 4, liste, wordLayout);      
  }

  //„ten“, „fifteen“, „half“
  //10, 50; 15, 45; 30
  if(nMalFuenf==2 or nMalFuenf==10)
  {
    int wortMinute[3][2] = 
    {
      5,0,
      6,1,
      7,1
    };
    reiheLedsInListe(wortMinute, 3, liste, wordLayout);            
  }
  else if(nMalFuenf==3 or nMalFuenf==9)
  {
    int wortMinute[7][2] = 
    {
      3,1,
      4,1,
      4,0,    
      5,0,
      6,1,
      7,0,    
      7,1
    };
    reiheLedsInListe(wortMinute, 7, liste, wordLayout);            
  }
  else if(nMalFuenf==6)
  {
    int wortMinute[4][2] = 
    {
      0,2,
      1,2,
      2,2,
      3,2
    };
    reiheLedsInListe(wortMinute, 4, liste, wordLayout);      
  }
}

void reihePastOderToInListe(int nMalFuenf, int liste, int wordLayout)
{
  //past; to
  //1-5; 6-11
  
  if(nMalFuenf>=0 and nMalFuenf<7)
  { reiheWortPastInListe(liste,wordLayout); }
  else if(nMalFuenf>=7 and nMalFuenf<12)
  { reiheWortToInListe(liste,wordLayout); }
}



void reiheWortOnlineInListe(int liste, int wordLayout)
{  
  if(wordLayout==1)
  {
    int wortOnline[6][2] = 
    {
      5,3,
      6,3,
      5,4,
      6,4,
      7,4,
      7,5
    }; 
    reiheLedsInListe(wortOnline, 6, liste, wordLayout);
  }
  else if(wordLayout==2)
  {
    int wortOnline[2][2] = 
    {
      5,4,
      6,5
    }; 
    reiheLedsInListe(wortOnline, 2, liste, wordLayout);
  }
  else if(wordLayout==3)
  {
    int wortOnline[2][2] = 
    {
      1,3,
      1,4
    }; 
    reiheLedsInListe(wortOnline, 2, liste, wordLayout);
  }
}

void reiheWortOfflineInListe(int liste, int wordLayout)
{  
  if(wordLayout==1)
  {
    int wortOffline[7][2] = 
    {
      3,0,
      3,1,
      3,2,
      5,4,
      6,4,
      7,4,
      7,5
    }; 
    reiheLedsInListe(wortOffline, 7, liste, wordLayout);
  }
  else if(wordLayout==2)
  {
    int wortOffline[3][2] = 
    {
      3,0,
      3,1,
      3,2
    }; 
    reiheLedsInListe(wortOffline, 3, liste, wordLayout);
  }
  else if(wordLayout==3)
  {
    int wortOffline[3][2] = 
    {
      3,0,
      3,1,
      3,2
    }; 
    reiheLedsInListe(wortOffline, 3, liste, wordLayout);
  }
}

void reiheWortWifiInListe(int liste, int wordLayout)
{  
  int wortWifi[4][2] = 
  {
    6,0,
    7,0,
    3,1,
    4,1
  }; 
  reiheLedsInListe(wortWifi, 4, liste, wordLayout);
}
