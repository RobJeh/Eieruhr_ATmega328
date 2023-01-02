#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitops.h"

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

//********************************************************************************
//      Definition von Variablen
//********************************************************************************

/* Display: Namen für Bits */
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


/* Alarm: Variablen */
#define LED1 BIT(5)
#define SPEAKER BIT(3)

#define Tonlaenge 150000
#define Pausenlaenge 0.75*Tonlaenge


/* Timer Interrupt: Variablen */
int Zeit_min = 1;
int Zeit_sec = 3;//Zeit_min*60;



//********************************************************************************
//      Bekanntmachung der Funktionen
//********************************************************************************

/*Display */
void Init_Ports();
void UmwandlungZahlen(int wert_timer);
void ANZ1(unsigned char Muster);
void ANZ2(unsigned char Muster);
void ANZ3(unsigned char Muster);
void ANZ4(unsigned char Muster);
void AnzeigeZiffer(void);


/*  Alarm */
 void Init_Alarm();
 void Wait(unsigned long delay);
 void LED1_aus();
 void LED1_an();
 void Speaker_an();
 void Speaker_aus();
 void Ton(unsigned long Zeit_an, unsigned long Zeit_aus);
 void Refrain();
 void Strophe();
 void Melodie();
 


/*Timer */
//Initialisierung Timer 0, Register schreiben
void Init_T0();



//********************************************************************************
//      INIT
//********************************************************************************

//Initialisierung Display
void Init_Ports() {
  SET(DDRB, DATA);              /* alle drei Bit Output */
  SET(DDRD, (CLK | LATCH));
  CLEAR(PORTD, (CLK | LATCH)); /* SHIFT und LATCH Low */
}


//Initialisierung des Alarms
void Init_Alarm() {
  SET(DDRB, LED1);     // LED auf Output
  LED1_aus(); 

  SET(DDRD, SPEAKER);     // SPEAKER auf Output
  SET(PORTD, SPEAKER);   // SPEAKER AUS
}


//Initialisierung des Timers
void Init_T0() {
  TCCR0A = 0x02;
  TCCR0B = 0x03;
  OCR0A = 250;     //Zählweite im CTC-Mode bis 250
  TIMSK0 = 0x03;
  TCNT0 = 0;    
  TIFR0 = 0x07;
  Serial.begin(9600);
}



//********************************************************************************
//      Timer
//      ISR für Timer
//********************************************************************************
ISR(TIMER0_COMPA_vect){
  //static-Variable wird bei Deklaration mit Wert definiert und bleibt auch bei Verlassen erhalten
  static int milliseconds = 0;
  if(milliseconds >= 1000){   //Fehlerbehandlung
    milliseconds = 1000;
  } else {
    milliseconds ++;  
  }
  if (Zeit_sec == 0){
    Melodie();
  }
  if(milliseconds == 1000 && Zeit_sec > 0){
    --Zeit_sec;
    Serial.println("Sekunden übrig:");
    Serial.println(Zeit_sec);
    milliseconds = 0;
  }
}


//********************************************************************************  
//        Alarm
//********************************************************************************
//LED 1 anschalten
void LED1_aus(){
  SET(PORTB, LED1);
}
//LED 1 ausschalten
void LED1_an(){
  CLEAR(PORTB, LED1);
}
//Lautsprecher anschalten
void Speaker_an(){
  CLEAR(PORTD, SPEAKER);
  LED1_an();
}
//Lautsprecher ausschalten
void Speaker_aus(){
  SET(PORTD, SPEAKER);
  LED1_aus();
}

//Funktion zum Erzeugen einer Verzögerung
void Wait(unsigned long delay) {
  for (volatile unsigned long i = 0; i < delay; i++)   // zähle bis delay -> Pause
     ;
}

//Erzeugen eines Tons
void Ton(unsigned long Zeit_an, unsigned long Zeit_aus) {
  Speaker_an();
  Wait(Zeit_an);
  Speaker_aus();
  Wait(Zeit_aus);
}

//Melodie als Refrain
void Refrain() {
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge, Pausenlaenge/4);
  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge, Pausenlaenge/4);
  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge*2, Pausenlaenge*2);
}
//Melodie als Strophe
void Strophe() {
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge, Pausenlaenge/2);
  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge/4, Pausenlaenge/4);
  Ton(Tonlaenge/4, Pausenlaenge/4);
  Ton(Tonlaenge/2, Pausenlaenge);

  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge/4, Pausenlaenge/4);
  Ton(Tonlaenge/4, Pausenlaenge/4);
  Ton(Tonlaenge/2, Pausenlaenge);
  

  Ton(Tonlaenge/2, Pausenlaenge/2);
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge, Pausenlaenge/2);
  Ton(Tonlaenge/4, Pausenlaenge/4);
  Ton(Tonlaenge, Pausenlaenge);
  Ton(Tonlaenge, Pausenlaenge/2);
  Ton(Tonlaenge/4, Pausenlaenge/4);
  Ton(Tonlaenge*2, Pausenlaenge);
}

//Melodie aus Refrain und Strophe
void Melodie() {
  for (volatile unsigned long i = 0; i < 1; i++) {
  Refrain();
  Refrain();
  Strophe();
  Strophe();
  Wait(Pausenlaenge*2);
  Serial.println("Melodie");
  }
  
}


//********************************************************************************  
//        Display
//        UMWANDELUNG DER ZAHLEN FÜR 4 SEGMENTE DER ANZEIGE
//********************************************************************************
void UmwandlungZahlen(int wert_timer){
  int wert_min = wert_timer / 60;
  int wert_sec = wert_timer % 60;
  int wert_min1 = wert_min / 10;
  int wert_min2 = wert_min % 10;
  int wert_sec1 = wert_sec / 10;
  int wert_sec2 = wert_sec % 10;  

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


//********************************************************************************
//      DARSTELLUNG DER ZAHLEN FÜR 4 DIGIT
//********************************************************************************

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



void AnzeigeZiffer(void) {
UmwandlungZahlen(Zeit_sec);

ANZ1(Muster1);
ANZ2(Muster2);
ANZ3(Muster3);
ANZ4(Muster4);
}




//********************************************************************************
//      Main()
//********************************************************************************
int main() {
  Init_Ports();
  Init_Alarm();
  Init_T0();
  sei();
  
  while(1) {
    AnzeigeZiffer();
  }
}
