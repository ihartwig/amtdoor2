#include "main.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
  {
    .Config =
      {
        .ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
        .DataINEndpoint           =
          {
            .Address          = CDC_TX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
          },
        .DataOUTEndpoint =
          {
            .Address          = CDC_RX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
          },
        .NotificationEndpoint =
          {
            .Address          = CDC_NOTIFICATION_EPADDR,
            .Size             = CDC_NOTIFICATION_EPSIZE,
            .Banks            = 1,
          },
      },
  };


// standard file streams for the serial ios
static FILE USBSerialStream;
// static FILE HWSerialStream;

/* Global Data */
char_buffer_t command_buffer;
char_buffer_t tag_buffer;
device_state_t device_state;
doorsense_state_t doorsense_current;
doorsense_state_t doorsense_last;
volatile uint16_t system_timer; // 1ms resolution, loops at 2^16=65536ms
uint16_t last_system_timer;

int main (void) {
  initBuffer(&command_buffer);
  initBuffer(&tag_buffer);

  setupHardware();

  /* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
  CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
  // Serial_CreateStream(&HWSerialStream);

  GlobalInterruptEnable();

  // start state machine
  device_state = IDLE;
  // default to OPEN door; could also mean no sensor
  doorsense_last = OPEN;
  doorsense_current = OPEN;

  last_system_timer = 0;

  while (1) {
    // update door sense
    doorsense_last = doorsense_current;
    if(DOORSENSE_PIN & _BV(DOORSENSE_PINN)) {
      /**
       * Because input is pulled up + NO switch, we expect to see a high signal
       * when door is open or when no sensor is available.
       */
      doorsense_current = OPEN;
    } else {
      doorsense_current = CLOSED;
    }
    // LED reflects door state; open=on
    if(doorsense_current == OPEN) {
      LEDPORT |= _BV(LEDPINN);
    } else {
      LEDPORT &= ~_BV(LEDPINN);
    }

    // periodically send a close door pulse in idle
    if(device_state == IDLE && system_timer >= DOOR_CLOSE_TIME_MS) {
      doorClose();
    }

    // during timeout, lock door if we see OPEN->CLOSE doorsense
    if(device_state == UNLOCKED &&
       doorsense_last == OPEN &&
       doorsense_current == CLOSED) {
      doorClose();
    }

    // after open timeout, go back to idle state
    if(device_state == UNLOCKED && system_timer >= DOOR_OPEN_TIME_MS) {
      doorClose();
    }

    // process any incomming serial commands (single bytes)
    while(CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface) > 0) {
      char new_command = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
      switch(new_command) {
        case 'D':
          doorOpen();
          break;
        case 'd':
          doorClose();
          break;
        default:
          // ignore
          break;
      }
    }

    // check for new data from RFID card reader, return when ready
    while(Serial_IsCharReceived()) {
      if(readOrDumpBuffer(
           (char)Serial_ReceiveByte(),
           &tag_buffer)) {
        if(strlen(tag_buffer.buffer) == TAG_LENGTH && device_state == IDLE) {
          fputs("TAG|", &USBSerialStream);
          fputs(&tag_buffer, &USBSerialStream);
          fputs("\r\n", &USBSerialStream);
        } else {
          fputs("BADREAD\r\n", &USBSerialStream);
        }
      }
    }

    // call maintenance functions
    CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
    USB_USBTask();
  }
  return 0;
}


void initBuffer(char_buffer_t *buffer) {
  buffer->buffer[0] = '\0';
  buffer->pos = 0;
  buffer->size = CHAR_BUFFER_SIZE;
}


/**
  Serial buffer control function.
  Attempt to read available characters into the buffer until either the buffer
  is full or a new line is reached. Omits newline, but adds null terminator.
  Return: true if complete line available.
**/
bool readOrDumpBuffer(char new_char, char_buffer_t *buffer) {
  // fprintf(&USBSerialStream, "char:%c,pos:%d\r\n", new_char, buffer->pos);
  if(new_char == '\r') {
    // ignore
    return false;
  } else if(new_char == '\n') {
    buffer->buffer[buffer->pos] = '\0';
    buffer->pos = 0;
    return true;
  } else {
    if(buffer->pos + 1 >= buffer->size) {
      // if no space for null terminator, this buffer is full.
      buffer->pos = 0;
    }
    buffer->buffer[buffer->pos] = new_char;
    buffer->pos++;
    return false;
  }
}


void setupHardware(void) {
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  // port init
  LEDDDR |= _BV(LEDPINN); //set up pin 0 on port B, off by default
  LEDPORT &= ~_BV(LEDPINN);

  RFIDEN_DDR |= _BV(RFIDEN_PINN); // pull RFID enable pin low
  RFIDEN_PORT &= ~_BV(RFIDEN_PINN);

  DOORCLOSE_DDR |= _BV(DOORCLOSE_PINN); // pull relay close side low
  DOORCLOSE_PORT &= ~_BV(DOORCLOSE_PINN);

  DOOROPEN_DDR |= _BV(DOOROPEN_PINN); // pull relay open side low
  DOOROPEN_PORT &= ~_BV(DOOROPEN_PINN);

  DOORSENSE_DDR &= ~_BV(DOORSENSE_PINN); // input with pull up
  DOORSENSE_PORT |= _BV(DOORSENSE_PINN);

  DEBUG_DDR |= DEBUG_PINN_ALL; // pull all debug pins low
  DEBUG_PORT &= ~DEBUG_PINN_ALL;

  // system time setup
  initSystemTime();

  // start serial interfaces
  Serial_Init(RFID_UART_BAUD, false);
  USB_Init();
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


void doorOpen() {
  // set device state/timer
  device_state = UNLOCKED;
  system_timer = 0;

  // disable keypad (also displays green)
  RFIDEN_PORT |= _BV(RFIDEN_PINN);

  // toggle relay open pin
  DOOROPEN_PORT |= _BV(DOOROPEN_PINN);
  _delay_ms(DOOR_RELAY_PULSE_MS);
  DOOROPEN_PORT &= ~_BV(DOOROPEN_PINN);
}


void doorClose() {
  // toggle relay close pin
  DOORCLOSE_PORT |= _BV(DOORCLOSE_PINN);
  _delay_ms(DOOR_RELAY_PULSE_MS);
  DOORCLOSE_PORT &= ~_BV(DOORCLOSE_PINN);

  // reenable keypad
  RFIDEN_PORT &= ~_BV(RFIDEN_PINN);

  // and assume device state/timer
  device_state = IDLE;
  system_timer = 0;
}


/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
  // LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}


/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
  // LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}


/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  bool ConfigSuccess = true;

  ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

  // LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}


/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
  CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}


ISR(TIMER0_COMPA_vect) {
  // figure out when the next compare should happen by doing a sine table lookup
  system_timer++;
}
