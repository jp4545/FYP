#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <TimerTwo.h>
#include <WProgram.h>
// allowed prescale factors
#define PS1    (1 << CS20)
#define PS8    (1 << CS21)
#define PS32   (1 << CS21) | (1 << CS20)
#define PS64   (1 << CS22)
#define PS128  (1 << CS22) | (1 << CS20)
#define PS256  (1 << CS22) | (1 << CS21)
#define PS1024 (1 << CS22) | (1 << CS21) | (1 << CS20)
// table by prescale = 2^n where n is the table index
static unsigned char preScale[] PROGMEM =  
  {PS1, 0, 0, PS8, 0, PS32, PS64, PS128, PS256, 0, PS1024};

bool TimerTwo::reset_;
void (*TimerTwo::f_)();
unsigned TimerTwo::period_;
//------------------------------------------------------------------------------
// initialize timer 2
unsigned char TimerTwo::init(unsigned usec, void (*f)(), bool reset) {
  f_ = f;
  reset_ = reset;
  // assume F_CPU is a multiple of 1000000
  // number of clock ticks to delay usec microseconds
  unsigned long ticks = usec * (F_CPU/1000000);
  // determine prescale factor and TOP/OCR2A value
  // use minimum prescale factor 
  unsigned char ps, i;
  for (i = 0; i < sizeof(preScale); i++) {
    ps = pgm_read_byte(&preScale[i]);
    if (ps && (ticks >> i) <= 256) break;
  }
  //return error if usec is too large
  if (i == sizeof(preScale)) return false;
  period_ = ((long)(ticks >> i) * (1 << i))/ (F_CPU /1000000);
//  Serial.println(i, DEC);
  // disable timer 2 interrupts
  TIMSK2 = 0;
  // use system clock (clkI/O).
  ASSR &= ~(1 << AS2);
  // Clear Timer on Compare Match (CTC) mode
  TCCR2A = (1 << WGM21);
  // only need prescale bits in TCCR2B
  TCCR2B = ps;
  // set TOP so timer period is (ticks >> i)
  OCR2A = (ticks >> i) - 1;
  return true;
}
//------------------------------------------------------------------------------
// Start timer two interrupts
void TimerTwo::start() {
  TIMSK2 |= (1 << OCIE2A);
}
//------------------------------------------------------------------------------
// Stop timer 2 interrupts
void TimerTwo::stop() {
  TIMSK2 = 0;
}
//------------------------------------------------------------------------------
// ISR for timer 2 Compare A interrupt
ISR(TIMER2_COMPA_vect) {
  // disable timer 2 interrupts
  TIMSK2 = 0;
  // call user function
	(*TimerTwo::f_)();
  // in case f_ enabled interrupts	
	cli();
	// clear counter if reset_ is true
	if (TimerTwo::reset_) {
	  // reset counter
    TCNT2 = 0;
    // clear possible pending interrupt
    TIFR2 |= (1 << OCF2A);
  }
	// enable timer 2 COMPA interrupt
  TIMSK2 |= (1 << OCIE2A);
}




