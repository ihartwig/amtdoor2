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


/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;
char USBSerialBuffer[256];


int main (void) {
  setupHardware();

  /* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
  CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

  GlobalInterruptEnable();

  LEDDDR = _BV(LEDPINN); //set up pin 0 on port B
  LEDPORT |= _BV(LEDPINN);

  while (1) {
    // PORTE |= _BV(6); //set pin 0 on port B high
    // _delay_ms(500);
    // PORTE &= ~_BV(6); //set pin 0 on port B low
    // _delay_ms(500);

    /* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
    // CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);

    // echo string
    uint16_t numBytes = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);
    if(numBytes > 0) {
      fgets(&USBSerialBuffer, 256, &USBSerialStream);
      fputs(&USBSerialBuffer, &USBSerialStream);
    }

    /* Echo all received data on the second CDC interface */
    // int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    // if (!(ReceivedByte < 0))
    //   CDC_Device_SendByte(&VirtualSerial_CDC_Interface, (uint8_t)ReceivedByte);

    // call maintenance functions
    CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
    USB_USBTask();
  }
  return 0;
}


void setupHardware(void) {
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);

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
