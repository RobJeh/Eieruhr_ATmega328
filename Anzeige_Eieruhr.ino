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

/* Namen fÃ¼r Bits */
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

#define DISP1 0xF1
#define DISP3 0xF4
#define DISP2 0xF2
#define DISP4 0xF8



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


void AnzeigeZiffer() {
 
  unsigned char Muster1 = ONE; //Anzeige 1 von links
  unsigned char Muster2 = TWO; //Anzeige 2 von links
  unsigned char Muster3 = THREE; //Anzeige 3 von links
  unsigned char Muster4 = FOUR; //Anzeige 4 von links
  /*unsigned char Stelle1 = DISP2;
  unsigned char Stelle2 = DISP1;
  unsigned char Stelle4 = DISP4;
  unsigned char Stelle3 = DISP3;*/
  

	ANZ1(Muster1);
  ANZ2(Muster2);
  ANZ3(Muster3);
  ANZ4(Muster4);


  /*for (int i = 0; i < 8; i++) {
      if (Muster1 & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Muster1 <<= 1;

      
      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }

    
    for (int i = 0; i < 8; i++) {
      if (Stelle1 & BIT(7)) {
        SET(PORTB, DATA);
      } else {
        CLEAR(PORTB, DATA);
      }
      Stelle1 <<= 1;

      SET(PORTD, CLK);
      CLEAR(PORTD, CLK);
    }*/


  

}

 

 
  

int main() {
  Init_Ports();

  while(1) {
    AnzeigeZiffer();
  }
}
 