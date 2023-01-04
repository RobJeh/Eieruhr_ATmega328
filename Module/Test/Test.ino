#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitops.h"

/* Define Bit fuer Taste 1-3 */
#define TA1 BIT(1)
#define TA2 BIT(2)
#define TA3 BIT(3)



#define LED1 BIT(5)
#define LED2 BIT(4)


/* Define Bit fuer Speaker */
#define SPEAKER BIT(3)

/* Define Bit fuer Display */
#define DATA BIT(0)
#define CLK BIT(7)
#define LATCH BIT(4)

/* Define Bit-Maske fuer Nummern */
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

/* Define Bit fuer Display-Segment */
#define DISP1 0xF1
#define DISP3 0xF4
#define DISP2 0xF2
#define DISP4 0xF8

unsigned char Muster1, Muster2, Muster3, Muster4;

int wert_timer = 586;
void Init_Anzeige()
{
  SET(DDRB, DATA);              /* alle drei Bit Output */
  SET(DDRD, (CLK | LATCH));
  CLEAR(PORTD, (CLK | LATCH)); /* SHIFT und LATCH Low */
}

void ANZ(unsigned char Muster, unsigned char DISP){
  //unsigned char Muster = Muster1;
  unsigned char Stelle = DISP;
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

void UmwandlungZahlen(int wert_timer){
  int wert_min = wert_timer / 60;
  int wert_sec = wert_timer % 60;
  int wert_min1 = wert_min / 10;
  int wert_min2 = wert_min % 10;
  int wert_sec1 = wert_sec / 10;
  int wert_sec2 = wert_sec % 10;  

  /*if ((wert_min1 && wert_min2) == 0){
    Muster1=BLACK;
    Muster2=BLACK;

  } 
  else{*/
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

void AnzeigeZiffer() {
UmwandlungZahlen(wert_timer);

ANZ(Muster1, DISP1);
ANZ(Muster2, DISP2);
ANZ(Muster3, DISP3);
ANZ(Muster4, DISP4);
}

int Zeit_min = 1;
int Zeit_sec = Zeit_min*60;

void Init_T0() {
  TCCR0A = 0x02;
  TCCR0B = 0x03;
  OCR0A = 250;     //Zählweite im CTC-Mode bis 250
  TIMSK0 = 0x03;
  TCNT0 = 0;    
  TIFR0 = 0x07;
  Serial.begin(115200);
}


ISR(TIMER0_COMPA_vect){
  //static-Variable wird bei Deklaration mit Wert definiert und bleibt auch bei Verlassen erhalten
  static int milliseconds = 0;

  milliseconds ++;
  if(milliseconds == 1000 && Zeit_sec > 0){
    Zeit_sec--;
    Serial.println("Sekunden übrig:");
    Serial.println(Zeit_sec);
    milliseconds = 0;
  }
}





void LED1_an(){
  CLEAR(PORTB, LED1);  // LED AN
}

void LED1_aus(){
  SET(PORTB, LED1);  // LED AUS
}
void LED2_an(){
  CLEAR(PORTB, LED2);  // LED AN
}

void LED2_aus(){
  SET(PORTB, LED2);  // LED AUS
}






/*Initialisiere PORTB Pin 2 und 5 auf Output */
void Init()
{
  
  
  SET(DDRB, LED1| LED2);     // LED auf Output
  SET(PORTB, LED1| LED2);   // LED AUS

  
  CLEAR(DDRC, TA1 | TA2 | TA3); /* Setze Tasten auf Input */
  SET(DDRD, SPEAKER);           /* SPEAKER auf Output */
  SET(PORTD, SPEAKER);          /* SPEAKER AUS */
}

void InitPCI()
{
  SET(PCMSK1, TA1);   /* erlaube der TA1 die auslösung des Interrupts */
  SET(PCMSK1, TA2);   /* erlaube der TA2 die auslösung des Interrupts */
  SET(PCMSK1, TA3);   /* erlaube der TA3 die auslösung des Interrupts */
  SET(PCICR, BIT(1)); /* Interrupt-Gruppe 1 (Portc) erlauben */
  SET(PCIFR, BIT(1)); /* anhängige Interrupts löschen */
}

void Init_T0() {
  TCCR0A = 0x02;
  TCCR0B = 0x03;
  OCR0A = 250;     //Zählweite im CTC-Mode bis 250
  TIMSK0 = 0x03;
  TCNT0 = 0;    
  TIFR0 = 0x07;
  Serial.begin(115200);
}

ISR(PCINT1_vect)
{
  Init_T0();
  //cli();              /* sperre Interrupts */
  // do something
  
  SET(PCIFR, BIT(1)); /* lösche alle Interrupts */
  sei();              /* Interrupts wieder freigeben */
}

ISR(TIMER0_COMPA_vect){
  //static-Variable wird bei Deklaration mit Wert definiert und bleibt auch bei Verlassen erhalten
  static int milliseconds = 0;

  milliseconds ++;
  if(milliseconds == 100){
    if (i == 0){
    LED1_an();
    i=1;
  } else {
    LED1_aus();
    i=0;
  }
    milliseconds = 0;
  }
}

int main()
{
  Init();
  //Init_Anzeige();
  InitPCI();
  sei();
  Init_Ports();

  while(1) {
    AnzeigeZiffer();
  }
}
