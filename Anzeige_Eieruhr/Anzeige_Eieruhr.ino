#include <avr/io.h>

/* Ausgabe der Zahl '2' auf Stelle 2 der 7-Segmentanzeige */

/* Anschluss:
 *    Schiebedaten  Port B Bit 0
 *    Schiebetakt   Port D Bit 7
 *    Ãœbernahmetakt Port D Bit 4
 *    
 * Ansteuerung:
 *    - Beide Taktleitungen im Ruhezustand auf Low
 *    - Ausgabe von 2 8-Bit Worten als Schiebedaten, MSB First
 *    - nach Ausgabe jedes Bits ein Low-High-Low Impuls am Schiebetakt
 *    - nach Ausgabe der beiden Bytes ein Low-High-Low Impuls am Ãœbernahmetakt
 *    
 *    Byte 1    Byte 2
 *    Segmente  Stelle    Segmentbits 0 => Segment an
 *    -----------------   Stellenbit  1 => Stelle an
 *    hgfedcba xxxx3210
 *    
 *    Stelle 0 ist ganz rechts, 3 links
 *    
 *  Bsp.
 *    10100100 11110100   Segmente a, b, d, e und g der Stelle 2 leuchten
 *    
 *       a
 *      ---
 *   f | g | b
 *      ---
 *   e |   | c
 *      ---
 *       d
 */

 /* Makros zur Bitmanipulation */
#define BIT(b) (1<<b)
#define SET(x,y) (x |= y)
#define CLEAR(x,y) ( x &=~ y)

/* Namen für Bits */
#define DATA  BIT(0)
#define CLK   BIT(7)
#define LATCH BIT(4)

#define ZERO 0xC0
#define ONE 0xF9
#define TWO 0xA4
#define THREE 0xB0
#define FOUR 0x99
#define FIVE 0x92
#define SIX 0x82
#define SEVEN 0xF8
#define EIGHT 0x80
#define NINE 0x90
#define BLACK 0xFF

#define DISP1 0xF1
#define DISP3 0xF4
#define DISP2 0xF2
#define DISP4 0xF8

unsigned char Muster1, Muster2, Muster3, Muster4;

int wert_min = 3; //EIngabe der Testwerte für Minuten
int wert_sec = 22; //Eingabe der Testwerte für Sekunden

void Init_Ports() {
  SET(DDRB, DATA);              /* alle drei Bit Output */
  SET(DDRD, (CLK | LATCH));
  CLEAR(PORTD, (CLK | LATCH)); /* SHIFT und LATCH Low */
}

void ANZ1(unsigned char Muster){
  //unsigned char Muster = Muster1;
  unsigned char Stelle = DISP1;
  for (int i = 0; i < 8; i++) {
      if (Muster & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Muster <<= 1;

      
      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }

    
    for (int i = 0; i < 8; i++) {
      if (Stelle & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Stelle <<= 1;

      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }

    SET(PORTD, LATCH);
    CLEAR(PORTD, LATCH);
}

void ANZ2(unsigned char Muster){
  unsigned char Stelle = DISP2;
  for (int i = 0; i < 8; i++) {
      if (Muster & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Muster <<= 1;

      
      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }

    
    for (int i = 0; i < 8; i++) {
      if (Stelle & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Stelle <<= 1;

      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }
    SET(PORTD, LATCH);
    CLEAR(PORTD, LATCH);
}

void ANZ3(unsigned char Muster){
  unsigned char Stelle = DISP3;
  for (int i = 0; i < 8; i++) {
      if (Muster & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Muster <<= 1;

      
      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }

    
    for (int i = 0; i < 8; i++) {
      if (Stelle & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Stelle <<= 1;

      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }
    SET(PORTD, LATCH);
    CLEAR(PORTD, LATCH);
}

void ANZ4(unsigned char Muster){
  unsigned char Stelle = DISP4;
  for (int i = 0; i < 8; i++) {
      if (Muster & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Muster <<= 1;

      
      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }

    
    for (int i = 0; i < 8; i++) {
      if (Stelle & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Stelle <<= 1;

      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }
    SET(PORTD, LATCH);
    CLEAR(PORTD, LATCH);
}

void UmwandlungZahlen(int wert_min, int wert_sec){
  int wert_min1 = wert_min / 10;
  int wert_min2 = wert_min % 10;
  int wert_sec1 = wert_sec / 10;
  int wert_sec2 = wert_sec % 10;

  if ((wert_min1 && wert_min2) == 0){
    Muster1=BLACK;
    Muster2=BLACK;    
  } 
  else{
      switch(wert_min1){
        case 0: Muster1=ZERO; break;    
        case 1: Muster1=ONE; break;
        case 2: Muster1=TWO; break;
  	    case 3: Muster1=THREE; break;
        case 4: Muster1=FOUR; break;
        case 5: Muster1=FIVE; break;
        case 6: Muster1=SIX; break;
  	    case 7: Muster1=SEVEN; break;
        case 8: Muster1=EIGHT; break;
        case 9: Muster1=NINE; break;
      }
      switch(wert_min2){
        case 0: Muster2=ZERO; break;    
        case 1: Muster2=ONE; break;
        case 2: Muster2=TWO; break;
  	    case 3: Muster2=THREE; break;
        case 4: Muster2=FOUR; break;
        case 5: Muster2=FIVE; break;
        case 6: Muster2=SIX; break;
  	    case 7: Muster2=SEVEN; break;
        case 8: Muster2=EIGHT; break;
        case 9: Muster2=NINE; break;
      }
  }
     switch(wert_sec1){
        case 0: Muster3=ZERO; break;    
        case 1: Muster3=ONE; break;
        case 2: Muster3=TWO; break;
  	    case 3: Muster3=THREE; break;
        case 4: Muster3=FOUR; break;
        case 5: Muster3=FIVE; break;
        case 6: Muster3=SIX; break;
  	    case 7: Muster3=SEVEN; break;
        case 8: Muster3=EIGHT; break;
        case 9: Muster3=NINE; break;
      }
  switch(wert_sec2){
        case 0: Muster4=ZERO; break;    
        case 1: Muster4=ONE; break;
        case 2: Muster4=TWO; break;
  	    case 3: Muster4=THREE; break;
        case 4: Muster4=FOUR; break;
        case 5: Muster4=FIVE; break;
        case 6: Muster4=SIX; break;
  	    case 7: Muster4=SEVEN; break;
        case 8: Muster4=EIGHT; break;
        case 9: Muster4=NINE; break;
      }
}

void AnzeigeZiffer() {
UmwandlungZahlen(wert_min,wert_sec);

ANZ1(Muster1);
ANZ2(Muster2);
ANZ3(Muster3);
ANZ4(Muster4);
}


int main() {
  Init_Ports();

  while(1) {
    AnzeigeZiffer();
  }
}
 