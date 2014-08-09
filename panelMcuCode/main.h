#ifndef _MAIN_H_
#define _MAIN_H_


/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

#include "descriptors.h"

#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>


/* HW Setup */

//
#define LEDDDR  DDRE
#define LEDPORT PORTE
#define LEDPIN  PINE
#define LEDPINN 6

// RFID Enable & Red/Green LED
#define RFIDEN_DDR  DDRD
#define RFIDEN_PORT PORTD
#define RFIDEN_PIN  PIND
#define RFIDEN_PINN 7

// Door Control Relay
#define DOOROPEN_DDR  DDRD
#define DOOROPEN_PORT PORTD
#define DOOROPEN_PIN  PIND
#define DOOROPEN_PINN 5

#define DOORCLOSE_DDR  DDRD
#define DOORCLOSE_PORT PORTD
#define DOORCLOSE_PIN  PIND
#define DOORCLOSE_PINN 4

// Door Open/Close Sense
// NOTE: JP1.6 and JP1.7 need to be tied to GND
#define DOORSENSE_DDR  DDRD
#define DOORSENSE_PORT PORTD
#define DOORSENSE_PIN  PIND
#define DOORSENSE_PINN 6

// Extra/debug pins
#define DEBUG_DDR      DDRF
#define DEBUG_PORT     PORTF
#define DEBUG_PIN      PINF
#define DEBUG_PINN_ALL 0xF3

#define CHAR_BUFFER_SIZE 128
#define TAG_LENGTH 10
#define RFID_UART_BAUD 2400

// Periodic close re-assert
#define DOOR_CLOSE_TIME_MS 10000
// Timeout when door is read as open
#define DOOR_OPEN_TIME_MS 8000
// Pulse fed to door control relay
#define DOOR_RELAY_PULSE_MS 50


/* Data Structures */
typedef struct char_buffer_t {
  char buffer[CHAR_BUFFER_SIZE];
  size_t pos;
  size_t size;
} char_buffer_t;

/**
 * IDLE (non blocking) -> Wait for new serial data ->
 * VERIFYING (non blocking) -> Should get a door command? (not really used) ->
 * UNLOCKED (blocking) -> Open relay, disable reader, set 10s timer.
 * When in non-UNLOCKED states, pulse close signal every 10s.
 **/
typedef enum device_state_t {
  IDLE,
  VERIFYING,
  UNLOCKED
} device_state_t;

/**
 * Track the door sense state.
 **/
typedef enum doorsense_state_t {
  OPEN,
  CLOSED
} doorsense_state_t;

/* Function Prototypes: */
bool readOrDumpBuffer(char new_char, char_buffer_t *buffer);
void setupHardware(void);
void initSystemTime(void);
void doorOpen(void);
void doorClose(void);
// void RFIDEnable(void);
// void RFIDDisable(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);


#endif
