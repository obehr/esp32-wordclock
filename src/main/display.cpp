#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <string.h>
#include "FastLED.h"
#include "list.cpp"

//#define NUM_LEDS 512
#define NUM_LEDS 64
#define DATA_PIN_1 26
#define DATA_PIN_2 19
#define BRIGHTNESS  80
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

static const char TAG2[] = "display";

class Display {
  private:
    int16_t variant_led_numbering;
    int16_t variant_clockface;
    int16_t orientation;
    int16_t wort_its[3];
    int16_t wort_oclock[6];
    int16_t wort_past[4];
    int16_t wort_to[2];
    int16_t wort_min_five[4];
    int16_t wort_min_ten[3];
    int16_t wort_min_fifteen[7];
    int16_t wort_min_twenty[6];
    int16_t wort_min_half[4];
    int16_t wort_hour_one[3];
    int16_t wort_hour_two[3];
    int16_t wort_hour_three[5];
    int16_t wort_hour_four[4];
    int16_t wort_hour_five[4];
    int16_t wort_hour_six[3];
    int16_t wort_hour_seven[5];
    int16_t wort_hour_eight[5];
    int16_t wort_hour_nine[4];
    int16_t wort_hour_ten[3];
    int16_t wort_hour_eleven[6];
    int16_t wort_hour_twelve[6];

    int16_t matrix_leds[8][8];
    char led_letters[64];
    uint16_t led_colors[64];
    

    const char matrix_clockface[8][8] = 
    {
      {'i', 't', 's', 'o', 'f', 't', 'w', 'e'},
      {'n', 't', 'y', 'f', 'i', 'v', 'e', 'n'},
      {'h', 'a', 'l', 'f', 'p', 'a', 's', 't'},
      {'t', 'o', 's', 't', 'w', 'o', 'n', 'e'},
      {'e', 'i', 'g', 'h', 'e', 'l', 'i', 'n'},
      {'s', 'o', 'u', 'r', 's', 'e', 'v', 'e'},
      {'i', 'f', 'i', 'v', 'e', 't', 'e', 'n'},
      {'x', 'n', 'o', 'c', 'l', 'o', 'c', 'k'}
    };

    CRGBArray<64> matrix;

    List liste_aufbau;
    List liste_abbau;
    List liste_effekt;
    List liste_nachricht;

    void clearLists(void)
    {
      liste_abbau.clear();
      liste_aufbau.clear();
      liste_effekt.clear();
      liste_nachricht.clear();
    }

    

    char get_letter(int16_t led_id)
    {
        //ESP_LOGI(TAG2, "Uebersetze led id %d, in letter %d", led_id, led_letters[led_id]);
        return led_letters[led_id];
    }


    /*
    ####################
    ## Setup function ##
    ####################
    */
    void init_colors()
    {
      for(int i=0; i<64; i++)
      { led_colors[i] = 100;
      }
    }

    void init_words()
    {
        int16_t v = variant_clockface;
        
        wort_its[0] = matrix_leds[0][0];
        wort_its[1] = matrix_leds[1][0];
        wort_its[2] = matrix_leds[2][0];

        wort_oclock[0] = matrix_leds[2][7];
        wort_oclock[1] = matrix_leds[3][7];
        wort_oclock[2] = matrix_leds[4][7];
        wort_oclock[3] = matrix_leds[5][7];
        wort_oclock[4] = matrix_leds[6][7];
        wort_oclock[5] = matrix_leds[7][7];

        wort_past[0] = matrix_leds[4][2];
        wort_past[1] = matrix_leds[5][2];
        wort_past[2] = matrix_leds[6][2];
        wort_past[3] = (v==2) ? matrix_leds[6][3] : matrix_leds[7][2];

        wort_to[0] = matrix_leds[6][3];
        wort_to[1] = matrix_leds[7][2];
        
        wort_min_five[0] = matrix_leds[3][1];
        wort_min_five[1] = matrix_leds[4][1];
        wort_min_five[2] = matrix_leds[5][1];
        wort_min_five[3] = matrix_leds[6][1];

        wort_min_ten[0] = matrix_leds[5][0];
        wort_min_ten[1] = matrix_leds[6][1];
        wort_min_ten[2] = matrix_leds[7][1];

        wort_min_fifteen[0] = matrix_leds[3][1];
        wort_min_fifteen[1] = matrix_leds[4][1];
        wort_min_fifteen[2] = matrix_leds[4][0];
        wort_min_fifteen[3] = matrix_leds[5][0];
        wort_min_fifteen[4] = matrix_leds[6][1];
        wort_min_fifteen[5] = matrix_leds[7][0];
        wort_min_fifteen[6] = matrix_leds[7][1];

        wort_min_twenty[0] = matrix_leds[5][0];
        wort_min_twenty[1] = matrix_leds[6][0];
        wort_min_twenty[2] = matrix_leds[7][0];
        wort_min_twenty[3] = matrix_leds[0][1];
        wort_min_twenty[4] = matrix_leds[1][1];
        wort_min_twenty[5] = matrix_leds[2][1];

        wort_hour_one[0] = (v==0) ? matrix_leds[5][3] : (v==1) ? matrix_leds[5][4] : matrix_leds[5][4];
        wort_hour_one[1] = (v==0) ? matrix_leds[6][3] : (v==1) ? matrix_leds[6][5] : matrix_leds[5][5];
        wort_hour_one[2] = (v==0) ? matrix_leds[7][3] : (v==1) ? matrix_leds[7][6] : matrix_leds[5][6];

        wort_hour_two[0] = (v==0) ? matrix_leds[3][3] : (v==1) ? matrix_leds[3][3] : matrix_leds[4][3];
        wort_hour_two[1] = (v==0) ? matrix_leds[4][3] : (v==1) ? matrix_leds[4][3] : matrix_leds[5][3];
        wort_hour_two[2] = (v==0) ? matrix_leds[5][3] : (v==1) ? matrix_leds[5][4] : matrix_leds[5][4];

        wort_hour_three[0] = (v==0) ? matrix_leds[3][3] : (v==1) ? matrix_leds[4][4] : matrix_leds[4][3];
        wort_hour_three[1] = (v==0) ? matrix_leds[3][4] : (v==1) ? matrix_leds[4][5] : matrix_leds[4][4];
        wort_hour_three[2] = (v==0) ? matrix_leds[3][5] : (v==1) ? matrix_leds[5][6] : matrix_leds[4][5];
        wort_hour_three[3] = (v==0) ? matrix_leds[4][6] : (v==1) ? matrix_leds[6][6] : matrix_leds[5][6];
        wort_hour_three[4] = (v==0) ? matrix_leds[5][5] : (v==1) ? matrix_leds[7][6] : matrix_leds[6][5];

        wort_hour_four[0] = (v==0) ? matrix_leds[1][6] : (v==1) ? matrix_leds[2][6] : matrix_leds[0][4];
        wort_hour_four[1] = (v==0) ? matrix_leds[1][5] : (v==1) ? matrix_leds[3][6] : matrix_leds[0][5];
        wort_hour_four[2] = (v==0) ? matrix_leds[2][5] : (v==1) ? matrix_leds[4][6] : matrix_leds[0][6];
        wort_hour_four[3] = (v==0) ? matrix_leds[3][5] : (v==1) ? matrix_leds[5][6] : matrix_leds[0][7];

        wort_hour_five[0] = (v==0) ? matrix_leds[1][6] : (v==1) ? matrix_leds[1][3] : matrix_leds[3][3];
        wort_hour_five[1] = (v==0) ? matrix_leds[2][6] : (v==1) ? matrix_leds[1][4] : matrix_leds[2][4];
        wort_hour_five[2] = (v==0) ? matrix_leds[3][6] : (v==1) ? matrix_leds[1][5] : matrix_leds[2][5];
        wort_hour_five[3] = (v==0) ? matrix_leds[4][6] : (v==1) ? matrix_leds[1][6] : matrix_leds[3][6];

        wort_hour_six[0] = (v==0) ? matrix_leds[0][5] : (v==1) ? matrix_leds[0][5] : matrix_leds[1][5];
        wort_hour_six[1] = (v==0) ? matrix_leds[0][6] : (v==1) ? matrix_leds[1][4] : matrix_leds[1][6];
        wort_hour_six[2] = (v==0) ? matrix_leds[0][7] : (v==1) ? matrix_leds[2][3] : matrix_leds[2][6];

        wort_hour_seven[0] = (v==0) ? matrix_leds[4][5] : (v==1) ? matrix_leds[7][3] : matrix_leds[7][3];
        wort_hour_seven[1] = (v==0) ? matrix_leds[5][5] : (v==1) ? matrix_leds[7][4] : matrix_leds[7][4];
        wort_hour_seven[2] = (v==0) ? matrix_leds[6][5] : (v==1) ? matrix_leds[7][5] : matrix_leds[7][5];
        wort_hour_seven[3] = (v==0) ? matrix_leds[7][5] : (v==1) ? matrix_leds[7][6] : matrix_leds[7][6];
        wort_hour_seven[4] = (v==0) ? matrix_leds[7][6] : (v==1) ? matrix_leds[7][7] : matrix_leds[6][6];

        wort_hour_eight[0] = (v==0) ? matrix_leds[0][4] : (v==1) ? matrix_leds[0][4] : matrix_leds[2][3];
        wort_hour_eight[1] = (v==0) ? matrix_leds[1][4] : (v==1) ? matrix_leds[1][4] : matrix_leds[2][4];
        wort_hour_eight[2] = (v==0) ? matrix_leds[2][4] : (v==1) ? matrix_leds[2][4] : matrix_leds[3][4];
        wort_hour_eight[3] = (v==0) ? matrix_leds[3][4] : (v==1) ? matrix_leds[3][4] : matrix_leds[4][4];
        wort_hour_eight[4] = (v==0) ? matrix_leds[3][3] : (v==1) ? matrix_leds[4][4] : matrix_leds[4][3];

        wort_hour_nine[0] = (v==0) ? matrix_leds[6][3] : (v==1) ? matrix_leds[0][3] : matrix_leds[1][4];
        wort_hour_nine[1] = (v==0) ? matrix_leds[6][4] : (v==1) ? matrix_leds[1][4] : matrix_leds[2][4];
        wort_hour_nine[2] = (v==0) ? matrix_leds[7][4] : (v==1) ? matrix_leds[2][5] : matrix_leds[3][5];
        wort_hour_nine[3] = (v==0) ? matrix_leds[7][5] : (v==1) ? matrix_leds[3][5] : matrix_leds[3][6];

        wort_hour_ten[0] = (v==0) ? matrix_leds[5][6] : (v==1) ? matrix_leds[5][5] : matrix_leds[4][6];
        wort_hour_ten[1] = (v==0) ? matrix_leds[6][6] : (v==1) ? matrix_leds[6][6] : matrix_leds[5][6];
        wort_hour_ten[2] = (v==0) ? matrix_leds[7][6] : (v==1) ? matrix_leds[7][7] : matrix_leds[6][6];

        wort_hour_eleven[0] = (v==0) ? matrix_leds[4][4] : (v==1) ? matrix_leds[5][3] : matrix_leds[6][3];
        wort_hour_eleven[1] = (v==0) ? matrix_leds[5][4] : (v==1) ? matrix_leds[6][4] : matrix_leds[6][4];
        wort_hour_eleven[2] = (v==0) ? matrix_leds[5][5] : (v==1) ? matrix_leds[7][4] : matrix_leds[7][4];
        wort_hour_eleven[3] = (v==0) ? matrix_leds[6][5] : (v==1) ? matrix_leds[7][5] : matrix_leds[7][5];
        wort_hour_eleven[4] = (v==0) ? matrix_leds[6][6] : (v==1) ? matrix_leds[7][6] : matrix_leds[7][6];
        wort_hour_eleven[5] = (v==0) ? matrix_leds[7][6] : (v==1) ? matrix_leds[7][7] : matrix_leds[6][6];

        wort_hour_twelve[0] = (v==0) ? matrix_leds[3][3] : (v==1) ? matrix_leds[3][3] : matrix_leds[4][3];
        wort_hour_twelve[1] = (v==0) ? matrix_leds[4][3] : (v==1) ? matrix_leds[4][3] : matrix_leds[5][3];
        wort_hour_twelve[2] = (v==0) ? matrix_leds[4][4] : (v==1) ? matrix_leds[5][3] : matrix_leds[6][3];
        wort_hour_twelve[3] = (v==0) ? matrix_leds[5][4] : (v==1) ? matrix_leds[6][4] : matrix_leds[6][4];
        wort_hour_twelve[4] = (v==0) ? matrix_leds[6][5] : (v==1) ? matrix_leds[7][5] : matrix_leds[7][5];
        wort_hour_twelve[5] = (v==0) ? matrix_leds[7][5] : (v==1) ? matrix_leds[7][6] : matrix_leds[7][6];
    }

    void init_letters()
    {
        for(int i=0; i<8; i++)
        {
            for(int j=0; j<8; j++)
            {
                led_letters[ matrix_leds[i][j] ] = matrix_clockface[i][j];
            }
        }
    }

    void init_matrix(int variante)
    {
      ESP_LOGI(TAG2, "init matrix %d", variante);
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
            matrix_leds[xi][yi]= xi + yi*x;
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
            matrix_leds[xi][yi]= xi + yi*x;
          }
          yi++;
          for(int xi=0; xi<x; xi++)
          {
            matrix_leds[7-xi][yi]= xi + yi*x;
          }
        }
      }
    }

    void turnMatrix(int orientation)
    {
      if(orientation>0 && orientation <4)
      {
        ESP_LOGI(TAG2, "Drehe Matrix. Ergebnis:");
        int16_t matrixTemp[8][8];
        int x=8;
        int y=8;
        for(int yi=0; yi<y; yi++)
        {
          for(int xi=0; xi<x; xi++)
          {
            if(orientation==1) //turn once clockwise 90degree
            { matrixTemp[xi][yi] = matrix_leds[x-yi-1][xi]; }
            
            else if(orientation==2) //turn twice 90degree
            { matrixTemp[xi][yi] = matrix_leds[x-xi-1][y-yi-1]; }
            
            else if(orientation==3) //turn once counter clockwise 90degree
            { matrixTemp[xi][yi] = matrix_leds[yi][y-xi-1]; }
          }
        }
        for(int yi=0; yi<y; yi++)
        {
          ESP_LOGI(TAG2, "");
          for(int xi=0; xi<x; xi++)
          { 
            matrix_leds[xi][yi] = matrixTemp[xi][yi];
            ESP_LOGI(TAG2, "%d", matrix_leds[xi][yi]);
            ESP_LOGI(TAG2, " ");
          }
        }
      }
    }

    void setzeFarbe(uint16_t farbeLeds)
    {
      /*Serial.println();
      Serial.print("->Setze Farbe: ");
      Serial.print(farbeLeds);*/
      int16_t ledId;
      for(int i=0; i<liste_aufbau.length; i++)
      { 
        ledId=liste_aufbau.get_item(i);
        led_colors[ledId] = farbeLeds; 
      }
      liste_aufbau.clear();
    }
    /*
    ########################
    ## End setup function ##
    ########################
    */

    void reihe_leds_in_listen(int16_t wort[], int laenge, bool listen[]) {
        for(int i=0; i<laenge; i++) {
            int16_t led = wort[i];
            
            if(listen[0]) {
                if(led == -1 || liste_abbau.get_index(led) == -1)
                { liste_abbau.append(led); }
            }
            
            if(listen[1]) {
                if(led == -1 || liste_aufbau.get_index(led) == -1)
                { liste_aufbau.append(led); }
            }
            
            if(listen[2]) {
                liste_effekt.append(led);
            }
        }
    }

    

    int berechneHelligkeit(int schritt, int ledIndex, int schrittweite, int offset, int volleHelligkeit, bool aufbau)
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
        
        int normaleHelligkeit = 80;
        int maximaleHelligkeit = normaleHelligkeit*1.5;
        maximaleHelligkeit = (maximaleHelligkeit>255)?255:maximaleHelligkeit;
        int differenz = maximaleHelligkeit - normaleHelligkeit;
        int neueHelligkeit;
        //int led;
        int16_t ledId;
        int stepsFadeUp=5;
        int stepsFadeDown=25;
        int stepsDelay=3;
        //int runde;
        int startPoint;
        int breakPoint;
        int endPoint;
        int anzahlLeds = liste_effekt.length;
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
              ledId = liste_effekt.get_item(j);
              if(ledId!=-1)
              { matrix[ledId].setHSV( led_colors[ledId], 200, neueHelligkeit); }
            }
          }
          vTaskDelay( pdMS_TO_TICKS(50) );
          FastLED.show();
        }      
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
        //int aktuelleHelligkeit;
        int neueHelligkeit;
        int volleHelligkeit = 80;
        int16_t ledId;
        int offsetAufbau=50;
        int offsetAbbau=10;
        int ledIndexAbbau;
        int i=0;
        bool ausstehenderAufbau = true;
        bool ausstehenderAbbau = true;
        
        while(ausstehenderAufbau or ausstehenderAbbau)
        {
          ausstehenderAufbau = false;
          for(int j=0; j<liste_aufbau.length; j++)
          {     
            ledId = liste_aufbau.get_item(j);
            neueHelligkeit = berechneHelligkeit(i,j,schrittweiteAufbau,offsetAufbau,volleHelligkeit,true);
            if(neueHelligkeit<volleHelligkeit)
            { ausstehenderAufbau = true; }
            ledIndexAbbau = liste_abbau.get_index(ledId);
            if(ledIndexAbbau==-1)
            { matrix[ledId].setHSV( led_colors[ledId], 200, neueHelligkeit); }
            else {
              int helligkeitAbbau = berechneHelligkeit(i,ledIndexAbbau,schrittweiteAbbau,offsetAbbau,volleHelligkeit,false);
              if(neueHelligkeit>helligkeitAbbau)
              {
                liste_abbau.clear_item(ledIndexAbbau);
                matrix[ledId].setHSV( led_colors[ledId], 200, neueHelligkeit);
              }
            }
          }
          
          ausstehenderAbbau = false;
          for(int k=0; k<liste_abbau.length; k++)
          {
            ledId = liste_abbau.get_item(k);
            if(ledId>0)
            {
              neueHelligkeit = berechneHelligkeit(i,k,schrittweiteAbbau,offsetAbbau,volleHelligkeit,false);
              if(neueHelligkeit>0)
              { ausstehenderAbbau = true; }
              matrix[ledId].setHSV(led_colors[ledId], 200, neueHelligkeit);
            }
          }
          FastLED.show();
          vTaskDelay( pdMS_TO_TICKS(50) );
          i++;
        }  
    }

    void iteriereLedsInListe(int liste, int aktion, bool onebyone, int delay)
    {
      /*
      Aktion 0: ausschalten
      Aktion 1: einschalten
      Aktion 2: FadeToBlackBy128
      Aktion 3: grün färben
      Aktion 4: rot färben
      */

      /*Serial.println();
      Serial.print("Iteriere Liste ");
      Serial.print(liste);
      Serial.print(" für ");
      Serial.print((aktion==0)?"Aktion 0: ausschalten":(aktion==1)?"Aktion 1: einschalten":(aktion==2)?"Aktion 2: FadeToBlackBy128":(aktion==3)?"Aktion 3: grün färben":(aktion==4)?"Aktion 3: grün färben":"ungueltige Aktion");
      Serial.print((onebyone)?" onebyone Delay ":" Delay pro Durchgang ");
      Serial.print(delay);*/

      int ledId;
      int laengeListe = (liste==0)?liste_abbau.length:(liste==1)?liste_aufbau.length:(liste==2)?liste_effekt.length:(liste==3)?liste_nachricht.length:0;

      for(int i=0; i<laengeListe; i++)
      {
        ledId = (liste==0)?liste_abbau.get_item(i):(liste==1)?liste_aufbau.get_item(i):(liste==2)?liste_effekt.get_item(i):(liste==3)?liste_nachricht.get_item(i):-1;
        
        if(aktion==0) { matrix[ledId].setHSV(0,0,0); }
        else if(aktion==1) { matrix[ledId].setHSV(led_colors[ledId], 200, 100); }
        else if(aktion==2) { matrix[ledId].fadeToBlackBy(128); }
        else if(aktion==3) { matrix[ledId].setHSV(96, 200, 100); }
        else if(aktion==4) { matrix[ledId].setHSV(0, 200, 100); }
        
        if(onebyone) 
        { 
          FastLED.show();
          vTaskDelay( pdMS_TO_TICKS(150) );
        }
      }
      if(!onebyone) 
      { 
        FastLED.show();
        vTaskDelay( pdMS_TO_TICKS(150) );
      }
    }

    void reihe_dummies_in_listen(int anzahl, bool listen[]) {
        int16_t dummies[anzahl];
        for(int i=0; i<anzahl; i++)
        { dummies[i] = -1; }
        reihe_leds_in_listen(dummies, anzahl, listen);
    }
    


    void reihe_stunden_in_listen(int stunde, bool listen[]) 
    {
        switch(stunde) {
          case 0: reihe_leds_in_listen(wort_hour_twelve, 6, listen); break;
            case 1: reihe_leds_in_listen(wort_hour_one, 3, listen); break;
            case 2: reihe_leds_in_listen(wort_hour_two, 3, listen); break;
            case 3: reihe_leds_in_listen(wort_hour_three, 5, listen); break;
            case 4: reihe_leds_in_listen(wort_hour_four, 4, listen); break;
            case 5: reihe_leds_in_listen(wort_hour_five, 4, listen); break;
            case 6: reihe_leds_in_listen(wort_hour_six, 3, listen); break;
            case 7: reihe_leds_in_listen(wort_hour_seven, 5, listen); break;
            case 8: reihe_leds_in_listen(wort_hour_eight, 5, listen); break;
            case 9: reihe_leds_in_listen(wort_hour_nine, 4, listen); break;
            case 10: reihe_leds_in_listen(wort_hour_ten, 3, listen); break;
            case 11: reihe_leds_in_listen(wort_hour_eleven, 6, listen); break;
            default: ESP_LOGI(TAG2, "unhandled hour"); break;
        }

    }


    void reihe_minuten_in_listen(int nMalFuenf, bool listen[]) 
    {
      //0=0, 1=5, 2=10, 3=15, 4=20, 5=25, 6=30, 7=35, 8=40, 9=45, 10=50, 11=55

      //„twenty“
      //20, 25, 35, 40
      if(nMalFuenf==4 or nMalFuenf==5 or nMalFuenf==7 or nMalFuenf==8)
      {
        reihe_leds_in_listen(wort_min_twenty, 6, listen);
      }
      //„five“
      //5, 25, 35, 55
      if(nMalFuenf==1 or nMalFuenf==5 or nMalFuenf==7 or nMalFuenf==11)
      {
        reihe_leds_in_listen(wort_min_five, 4, listen);      
      }

      //„ten“, „fifteen“, „half“
      //10, 50; 15, 45; 30
      if(nMalFuenf==2 or nMalFuenf==10)
      {
        reihe_leds_in_listen(wort_min_ten, 3, listen);            
      }
      else if(nMalFuenf==3 or nMalFuenf==9)
      {
        reihe_leds_in_listen(wort_min_fifteen, 7, listen);            
      }
      else if(nMalFuenf==6)
      {
        reihe_leds_in_listen(wort_min_half, 4, listen);      
      }
    }

    void reihe_past_oder_to_in_listen(int nMalFuenf, bool listen[])
  {
    //past; to
    //1-5; 6-11
    
    if(nMalFuenf>=0 and nMalFuenf<7)
    { reihe_leds_in_listen(wort_past, 4, listen); }
    else if(nMalFuenf>=7 and nMalFuenf<12)
    { reihe_leds_in_listen(wort_to, 2, listen); }
  }

  public:
    int16_t mode = 0; //0 = wait, 1 = set time, 2 = animate
    Display(int data_pin, int variant, int orientation) {
      ESP_LOGI(TAG2, "construct display");
      init_matrix(variant);
      if(orientation >0 && orientation <4) {
        turnMatrix(orientation);
      }
      ESP_LOGI(TAG2, "created matrix %d", matrix[3][2]);
      init_colors();
      init_words();
      init_letters();

      
      FastLED.addLeds<LED_TYPE, DATA_PIN_1>(matrix, 64);
      FastLED.setMaxPowerInVoltsAndMilliamps(12,2000);
    }

    void loop_display()
    {
      while(true)
      {
        if(mode > 0)
        { 
          bearbeiteListe();
        }
        else if(mode == 0)
        { 
          ESP_LOGI(TAG2, "Command to stop animation. Confirm with -1");
          mode = -1; 
        }
          vTaskDelay( pdMS_TO_TICKS(2000) );
      }
    }
    
    void stelle_zeit(int letzte_stunde, int letzte_minute, int aktuelle_stunde, int aktuelle_minute) {
        ESP_LOGI(TAG2, "stelle Uhrzeit %d, %d, %d, %d", letzte_stunde, letzte_minute, aktuelle_stunde, aktuelle_minute);
        bool abbau[3] = {true, false, false};
        bool aufbau_und_effekt[3] = {false, true, true};
        bool effekt[3] = {false, false, true};
        bool alle[3] = {true, true, true};
        
        reihe_leds_in_listen(wort_its, 3, alle);

        reihe_dummies_in_listen(3, effekt);
        
        if(letzte_minute != 0) { 
            reihe_minuten_in_listen(letzte_minute, abbau);
            reihe_past_oder_to_in_listen(letzte_minute, abbau);
        }
        
        if(aktuelle_minute != 0) { 
            reihe_minuten_in_listen(aktuelle_minute, aufbau_und_effekt);
            reihe_dummies_in_listen(3, effekt);
            reihe_past_oder_to_in_listen(aktuelle_minute, aufbau_und_effekt);
            reihe_dummies_in_listen(3, effekt);
        }

        reihe_stunden_in_listen(letzte_stunde, abbau);
        reihe_stunden_in_listen(aktuelle_stunde, aufbau_und_effekt);

        reihe_dummies_in_listen(3, effekt);
        
        if(letzte_minute == 0) { 
            reihe_leds_in_listen(wort_oclock, 6, abbau); 
        }
        
        if(aktuelle_minute == 0) { 
            reihe_leds_in_listen(wort_oclock, 6, aufbau_und_effekt); 
        }
    }

    void bearbeiteListe()
    {
      ESP_LOGI(TAG2, "bearbeite Liste %d", mode);
      if(mode==1) //schalten
      { 
        iteriereLedsInListe(0,0,true,100);
        iteriereLedsInListe(1,1,true,100);
        liste_aufbau.clear();
        liste_abbau.clear();
        mode = -1;
      }
      if(mode==2) //fade over
      {
        fadeLeds();
        liste_aufbau.clear();
        liste_abbau.clear();
        mode = -1;
      }
      else if(mode==3) //animate leds in list 3
      {
        flashLeds();
      }
      ESP_LOGI(TAG2, "Ende: bearbeite Liste %d", mode);
    }

    void show() {
        FastLED.show();
    }
};