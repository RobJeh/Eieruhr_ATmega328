#include <avr/io.h>

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
  static int milliseconds = 0;

  milliseconds ++;
  if(milliseconds == 1000 && Zeit_sec > 0){
    Zeit_sec--;
    Serial.println("Sekunden übrig:");
    Serial.println(Zeit_sec);
    milliseconds = 0;
  }
}



int main() { 
  Init_T0();
  sei();
  for(;;);
}
