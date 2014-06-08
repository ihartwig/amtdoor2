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

  LEDDDR = _BV(LEDPINN); //set up pin 0 on port B
  LEDPORT |= _BV(LEDPINN);

  RFIDEN_DDR = _BV(RFIDEN_PINN); // pull RFID enable pin low
  RFIDEN_PORT &= ~_BV(RFIDEN_PINN);


  DDRD |= _BV(6);
  last_system_timer = 0;

  while (1) {
    // PORTE |= _BV(6); //set pin 0 on port B high
    // _delay_ms(500);
    // PORTE &= ~_BV(6); //set pin 0 on port B low
    // _delay_ms(500);

    if(system_timer + 1000 >= last_system_timer) {
      fprintf(&USBSerialStream, "timer: %dms\n", system_timer);
      if(system_timer < 60000) {
        last_system_timer += 1000;
      } else {
        system_timer = 0;
        last_system_timer = 0;
      }
    }

    // echo command string
    // while(CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface) > 0) {
    //   if(readOrDumpBuffer(
    //        (char)CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface),
    //        &command_buffer)) {
    //     fputs(&command_buffer, &USBSerialStream);
    //   }
    // }

    // process any incommming serial commands
    while(CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface) > 0) {
      if(readOrDumpBuffer(
           (char)CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface),
           &command_buffer)) {
        if(strcmp("D", &command_buffer)) {
          // open door

        }

        // no other commands implemented here
      }
    }

    // check for new data from RFID card reader, return when ready
    while(Serial_IsCharReceived()) {
      if(readOrDumpBuffer(
           (char)Serial_ReceiveByte(),
           &tag_buffer)) {
        if(strlen(tag_buffer.buffer) == TAG_LENGTH) {
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

  initSystemTime();

  // start hw serial
  Serial_Init(RFID_UART_BAUD, false);

  // start virtual serial
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
  PORTD |= _BV(6);
  system_timer++;
  PORTD &= ~_BV(6);
}
