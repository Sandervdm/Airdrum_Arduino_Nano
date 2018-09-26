/*
 * functies.h
 *
 * Created: 3-5-2017 13:08:08
 *  Author: Sander
 */ 


#ifndef FUNCTIES_H_
#define FUNCTIES_H_

#include <avr/io.h>
#include "main.h"

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} RGB_Data;

extern void StripShow(void);

extern void setColor(RGB_Data *LEDS, uint8_t* data);

extern void updateLeds(RGB_Data *LEDS);

extern void UART_init(void);

extern unsigned char UART_get(void);

extern void UART_send(unsigned char data);

extern void UART_send_str(char* str);

extern void SetTimer1Delay33ms(void);

extern unsigned char Timer1Empty(void);

extern void adc_init(void);

extern uint8_t adc_read(uint8_t ch);

extern void send_hand_data(void);

extern void Prot_Send(uint8_t* ptr, uint8_t length, uint8_t type);

#endif /* FUNCTIES_H_ */