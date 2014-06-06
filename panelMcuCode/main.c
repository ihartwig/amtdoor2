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

  while (1) {
    // PORTE |= _BV(6); //set pin 0 on port B high
    // _delay_ms(500);
    // PORTE &= ~_BV(6); //set pin 0 on port B low
    // _delay_ms(500);

    // echo command string
    while(CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface) > 0) {
      if(readOrDumpBuffer(
           (char)CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface),
           &command_buffer)) {
        fputs(&command_buffer, &USBSerialStream);
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

  // start hw serial
  Serial_Init(RFID_UART_BAUD, false);

  // start virtual serial
  USB_Init();
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
