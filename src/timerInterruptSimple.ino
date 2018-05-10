/*
   This program uses interrupts for (longer) timing tasks in the seconds range.

   @see: https://arduino-projekte.webnode.at/registerprogrammierung/timer-interrupt/
   @see: http://www.gammon.com.au/interrupts
   @see: http://www.gammon.com.au/timers
   @see: http://www.instructables.com/id/Arduino-Timer-Interrupts/
   @see: http://maxembedded.com/2011/07/avr-timers-ctc-mode/
 * */
// ATTENTION: Here TIMER 1 is used which is a 16-bit timer. Thus the compare value cannot be greater than 65535, otherwise the timer would overflow.
//            So the maximum time between two interrupts with a cpu frequency of 16000000 hz can only be:
//                OCR1A = CMP_VALUE = INTERRUPT_INTERVAL * TICKS_PER_SEC - 1
//                => INTERRUPT_INTERVAL = CMP_VALUE / (TICKS_PER_SEC - 1)
//                => INTERRUPT_INTERVAL = CMP_VALUE / (( CPU_FREQ / PRESCALER ) - 1)
//                => INTERRUPT_INTERVAL = 65535 / (( 16000000 / 1024 ) - 1)
//                => INTERRUPT_INTERVAL = 4.1945 seconds

#define CPU_FREQ 16000000L    // cpu clock from boards.txt
#define PRESCALER 1024        // cpu prescaler

const unsigned long TICKS_PER_SEC = ( CPU_FREQ / PRESCALER ); // base for calculation of compare value for output compare register OCR1A
const unsigned short INTRPT_INTERVALL = 4;   // interrupt every 4 seconds
const unsigned short CMP_VALUE = INTRPT_INTERVALL * TICKS_PER_SEC - 1; // compare value for OCR1A: = amount of ticks in an interrupt intervall

volatile unsigned long counter = 0;
volatile unsigned long cmp = 0;


void setup()
{
  Serial.begin(9600);
  Serial.print("CPU_FREQ:          "); Serial.println(CPU_FREQ);
  Serial.print("PRESCALER:         "); Serial.println(PRESCALER);
  Serial.print("INTRPT_INTERVALL:  "); Serial.println(INTRPT_INTERVALL);
  Serial.print("TICKS_PER_SEC:     "); Serial.println(TICKS_PER_SEC);
  Serial.print("CMP_VALUE:         "); Serial.println(CMP_VALUE);

  cli();                               // disable global interrupts
  TCNT1 = 0;                           // delete timer counter register
  TCCR1A = 0;                          // delete TCCR1A-Registers
  TCCR1B = 0;                          // delete TCCR1B-Registers
  TCCR1B |= (1 << WGM12);              // CTC-Mode (Waveform Generation Mode): resets TCNT1 to0 after interrupt
  TCCR1B |= (1 << CS12) | (1 << CS10); // set prescaler to 1024: CS10 und CS12 (Clock Select)
  OCR1A = CMP_VALUE;                   // set compare value
  TIMSK1 |= (1 << OCIE1A);             // enable interrupt: set output compare interrupt enable for 1A
  sei();                               // enable global interrupts
}

// Interrupt Service Routine for Timer 1
ISR(TIMER1_COMPA_vect)
{
  ++counter;
}

void loop()
{
  if (cmp < counter)
  {

    Serial.print(counter); Serial.print(": "); Serial.println( millis() );
    ++cmp;
  }
}
