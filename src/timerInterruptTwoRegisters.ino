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

volatile unsigned long counterA = 0;
volatile unsigned long counterB = 0;
volatile unsigned short cntQuarter = 1;
unsigned long cntCmpA = 0;
unsigned long cntCmpB = 0;


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
  TCCR1B |= (1 << WGM12);              // CTC-Mode (Waveform Generation Mode): resets TCNT1 to0 after interrupt, makes OCR1A the leading compare register
  TCCR1B |= (1 << CS12) | (1 << CS10); // set prescaler to 1024: CS10 und CS12 (Clock Select)
  OCR1A = CMP_VALUE;                   // set compare value
  OCR1B = CMP_VALUE / 4;               // a second comapare value in OCR1B can be used as long as its value is lower than that of OCR1A
  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B); // enable interrupts: set output compare interrupt enable for 1A and 1B
  sei();                               // enable global interrupts
}

// Interrupt Service Routine for timer 1 OCR1A
ISR(TIMER1_COMPA_vect)
{
  OCR1B = (CMP_VALUE / 4) * cntQuarter;
  if (cntQuarter < 3) ++cntQuarter;
  else cntQuarter = 1;

  // OCR1B = CMP_VALUE+1;   // disable interrupt only one time by setting it higher than the value in OCR1A
  ++counterA;
}

// Interrupt Service Routine for timer 1 OCR1B
ISR(TIMER1_COMPB_vect)
{
  ++counterB;
}


void loop()
{

  if (cntCmpA != counterA)    // use != for comparsison and not < because it will give problem when unsigned long overlows
  {
    Serial.println();
    Serial.print(counterA); Serial.print("A: "); Serial.println( millis() / 1000 );
    cntCmpA = counterA;
  }

  if (cntCmpB != counterB)
  {
    Serial.print(counterB); Serial.print("B: "); Serial.println( millis() / 1000 );
    cntCmpB = counterB;
  }
}



