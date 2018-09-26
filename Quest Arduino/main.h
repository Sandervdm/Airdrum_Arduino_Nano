/*
 * main.h
 *
 * Created: 3-5-2017 13:07:11
 *  Author: Sander
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include <string.h>
#include <avr/io.h>

#include "functies.h"

// LED pin data
#define LED_DDR			DDRB
#define LED_PORT		PORTB
#define LED_PIN			0b00100000

// UART macro
#define UART_HAS_DATA()	UCSR0A & (1<<RXC0)

// panel data
#define PANEL_COUNT		6

// protocol defines
#define PROT_PREAMBLE	85
#define PROT_DATA_RGB	10
#define PROT_DATA_HAND	11
#define PROT_NACK		20
#define PROT_ACK		21

// Ledstrip data
#define NUM_PIXELS		PANEL_COUNT * 8  // 8 LEDs per panel, total of 5 * 8 leds
#define NUM_BYTES		NUM_PIXELS * 3 // 3 bytes per LED (R G B), total of 48 * 3 bytes



#endif /* MAIN_H_ */