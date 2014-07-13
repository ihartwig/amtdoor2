#include "main.h"
/**
 * IDLE (non blocking) -> Wait for new serial data ->
 * VERIFYING (non blocking) -> Should get a door command? ->
 * OPEN (blocking) -> Open relay, disable reader, set 10s timer.
 * When in other states, pulse close signal every 10s.
 **/





/* Global Data */
device_state_t device_state;
volatile uint16_t system_timer; // 1ms resolution, loops at 2^16=65536ms
uint16_t last_system_timer;

int main (void) {
  setupHardware();

  GlobalInterruptEnable();

  // debug init
  DDRF |= _BV(6); // interrupt monitor
  DDRF |= _BV(5); // system loop monitor
  last_system_timer = 0;

  while (1) {
    PORTF |= _BV(5);
    _delay_ms(50);
    PORTF &= ~_BV(5);
    _delay_ms(50);
  }
  return 0;
}

void setupHardware(void) {
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  // port init
  LEDDDR |= _BV(LEDPINN); //set up pin 0 on port B
  LEDPORT |= _BV(LEDPINN);

  RFIDEN_DDR |= _BV(RFIDEN_PINN); // pull RFID enable pin low
  RFIDEN_PORT &= ~_BV(RFIDEN_PINN);

  DOORCLOSE_DDR |= _BV(DOORCLOSE_PINN); // pull relay close side low
  DOORCLOSE_PORT &= ~_BV(DOORCLOSE_PINN);

  DOOROPEN_DDR |= _BV(DOOROPEN_PINN); // pull relay open side low
  DOOROPEN_PORT &= ~_BV(DOOROPEN_PINN);

  // system time setup
  initSystemTime();
}


/**
Configure TIMER0 to raise an interrupt every 1ms and increment the system_time
variable.
**/
void initSystemTime() {
  // F_CPU = 16000000
  // F_Timer1 = 16000000/64 = 250000
  // 1ms @ 250000 hz = 250
  // configure TIMER0 to raise an interrupt every 1ms

  // set timer for no scaling Fovr = 16000000/256 = 62500Hz, CTC Mode
  TCCR0A |= _BV(WGM01);
  TCCR0B |= _BV(CS01) | _BV(CS00);
  TIMSK0 |= _BV(OCIE0A);
  OCR0A = 250;
}


// void doorOpen() {
//   // set device state/timer
//   device_state = OPEN;
//   system_timer = 0;

//   // disable keypad (also displays green)
//   RFIDEN_PORT |= _BV(RFIDEN_PINN);

//   // toggle relay open pin
//   DOOROPEN_PORT |= _BV(DOOROPEN_PINN);
//   _delay_ms(50);
//   DOOROPEN_PORT &= ~_BV(DOOROPEN_PINN);
// }


// void doorClose() {
//   // toggle relay close pin
//   DOORCLOSE_PORT |= _BV(DOORCLOSE_PINN);
//   _delay_ms(50);
//   DOORCLOSE_PORT &= ~_BV(DOORCLOSE_PINN);

//   // reenable keypad
//   RFIDEN_PORT &= ~_BV(RFIDEN_PINN);

//   // and assume device state/timer
//   device_state = IDLE;
//   system_timer = 0;
// }


ISR(TIMER0_COMPA_vect) {
  // figure out when the next compare should happen by doing a sine table lookup
  PORTF |= _BV(6);
  system_timer++;
  PORTF &= ~_BV(6);
}
