#include <avr/io.h>

/* Makros*/
#define BIT(n)  (1<< n)
#define SET(x, n) (x |= n)
#define CLEAR(x, n)  (x &= ~n)
#define TOGGLE(x, n) (x ^= n)

#define LED1 BIT(5)

#define SW1 BIT(1)
#define SW2 BIT(2)
#define SW3 BIT(3)

#define SPEAKER BIT(3)

#define Tonlaenge 150000
#define Pausenlaenge 0.75*Tonlaenge



 /* Prototypen */
 void Init();
 void Wait(unsigned long delay);

void LED1_aus(){
  SET(PORTB, LED1);
}
void LED1_an(){
  CLEAR(PORTB, LED1);
}
void Speaker_an(){
  CLEAR(PORTD, SPEAKER);
  LED1_an();
  
}
void Speaker_aus(){
  SET(PORTD, SPEAKER);
  LED1_aus();
}

int Ta(int mask){
  if (PINC & mask)
    return 0;
   else
    return 1;
}


/* Wait Funktion */
void Wait(unsigned long delay) {
  for (volatile unsigned long i = 0; i < delay; i++)   // zähle bis delay -> Pause
     ;
}


/*Initialisiere PORTB Pin 2 und 5 auf Output */
void Init() {
  SET(DDRB, LED1);     // LED auf Output
  LED1_aus();

  CLEAR(DDRC, SW1 | SW2 | SW3);  // Setze Tasten auf Input 

  SET(DDRD, SPEAKER);     // SPEAKER auf Output
  SET(PORTD, SPEAKER);   // SPEAKER AUS
}

void Ton(unsigned long Zeit_an, unsigned long Zeit_aus) {
  Speaker_an();
  Wait(Zeit_an);
  Speaker_aus();
  Wait(Zeit_aus);
}

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

void Melodie() {
  for (volatile unsigned long i = 0; i < 1; i++) {
  Refrain();
  Refrain();
  Strophe();
  Strophe();
  Wait(Pausenlaenge*2);
  }
  
}


int main()  {
  Init();
  while(1) {        // forever
    if (Ta(SW1)) {
      Melodie();
    }

  }
}
