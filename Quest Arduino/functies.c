/*
 * functies.c
 *
 * Created: 3-5-2017 13:08:21
 *  Author: Sander
 */ 

 #include "functies.h"

 volatile uint8_t *port   = &LED_PORT;
 volatile uint8_t pinMask = LED_PIN;
 volatile uint8_t RGBVAL[NUM_BYTES];

 //////////////////////////////////////////////////////////////////////////
 // ws2812 protocol bitbanged
 void StripShow(void)
 {

	 volatile uint16_t i   = NUM_BYTES;			// Loop counter
	 volatile uint8_t *ptr = RGBVAL;			// Pointer to next byte
	 volatile uint8_t b    = *ptr++;			// Current byte value
	 volatile uint8_t hi   = *port |  pinMask;	// PORT w/output bit set high
	 volatile uint8_t lo   = *port & ~pinMask;	// PORT w/output bit set low
	 volatile uint8_t next = lo;				// Start value for port
	 volatile uint8_t bit  = 8;					// Bitcounter

	 asm volatile(
	 "head20:"                  "\n\t" // Clk  Pseudocode    (T =  0)
	 "st   %a[port],  %[hi]"    "\n\t" // 2    PORT = hi     (T =  2)
	 "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
	 "mov  %[next], %[hi]"      "\n\t" // 0-1   next = hi    (T =  4)
	 "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
	 "st   %a[port],  %[next]"  "\n\t" // 2    PORT = next   (T =  7)
	 "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  8)
	 "breq nextbyte20"          "\n\t" // 1-2  if(bit == 0) (from dec above)
	 "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 10)
	 "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 12)
	 "nop"                      "\n\t" // 1    nop           (T = 13)
	 "st   %a[port],  %[lo]"    "\n\t" // 2    PORT = lo     (T = 15)
	 "nop"                      "\n\t" // 1    nop           (T = 16)
	 "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 18)
	 "rjmp head20"              "\n\t" // 2    -> head20 (next bit out)
	 "nextbyte20:"              "\n\t" //                    (T = 10)
	 "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 11)
	 "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 13)
	 "st   %a[port], %[lo]"     "\n\t" // 2    PORT = lo     (T = 15)
	 "nop"                      "\n\t" // 1    nop           (T = 16)
	 "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 18)
	 "brne head20"              "\n"   // 2    if(i != 0) -> (next byte)
	 : [port]  "+e" (port),			   // Variables in assemblye are connected to C variables
	 [byte]  "+r" (b),
	 [bit]   "+r" (bit),
	 [next]  "+r" (next),
	 [count] "+w" (i)
	 : [ptr]    "e" (ptr),
	 [hi]     "r" (hi),
	 [lo]     "r" (lo)
	 );
 }

 //////////////////////////////////////////////////////////////////////////
 // The received data is transfered into the data structures
 void setColor(RGB_Data *LEDS, uint8_t* data)
 {
	 for(uint8_t i = 0; i < PANEL_COUNT; i++) {
		 LEDS[i].red = data[i*4];
		 LEDS[i].green = data[(i*4)+1];
		 LEDS[i].blue = data[(i*4)+2];
		 LEDS[i].alpha = data[(i*4)+3];
	 }
 }

 //////////////////////////////////////////////////////////////////////////
 // The data structures are read and LEDRAM is filled with this data
 void updateLeds(RGB_Data *LEDS)
 {
	 for(uint8_t i = 0; i < PANEL_COUNT; i++)
	 {
		 uint16_t red = (LEDS[i].red * LEDS[i].alpha) / 255;
		 uint16_t green = (LEDS[i].green * LEDS[i].alpha) / 255;
		 uint16_t blue = (LEDS[i].blue * LEDS[i].alpha) / 255;
		 uint8_t offset = 24 * i;
		 for(uint8_t j=0;j<8;j++)
		 {
			 RGBVAL[offset + (j*3)] = red;
			 RGBVAL[offset + 1 + (j*3)] = green;
			 RGBVAL[offset + 2 + (j*3)] = blue;
		 }
	 }
 }

 //////////////////////////////////////////////////////////////////////////
 // uart functions
 void UART_init(void)
 {
	 UBRR0H = 0;
	 UBRR0L = 103; //UBRR = (clk / (16 * BAUD)) -1 
	 UCSR0B |= ((1<<TXEN0)|(1<<RXEN0));
	 UCSR0C |= ((1<<UCSZ00)|(1<<UCSZ01));
 }

 unsigned char UART_get(void)
 {
	 while(!(UCSR0A & (1<<RXC0)));
	 return UDR0;
 }

 void UART_send(unsigned char data)
 {
	 while(!(UCSR0A & (1 << UDRE0)));
	 UDR0 = data;
 }

 void UART_send_str(char* str)
 {
	 while(*str != 0){
		 UART_send(*str);
		 str++;
	 }
 }

 //////////////////////////////////////////////////////////////////////////
 // timer functions
 void SetTimer1Delay33ms(void)
 {
	 //disable the timer
	 TCCR1B &= 0b11111000;
	 //reset the overflow flag
	 TIFR1 |= (1 << OCF1A);
	 // Set the Timer Mode to CTC
	 TCCR1B |= (1 << WGM12);
	 // Set the value that you want to count to
	 OCR1A = 469;
	 TCNT1 = 0;
	 // set prescaler to 1024 and start the timer
	 TCCR1B |= (1 << CS12) | (1 << CS10);
 }

 unsigned char Timer1Empty(void)
 {
	 if(TIFR1 & (1 << OCF1A)) {
		 TCCR1B &= 0b11111000;
		 return 1;
	 }
	 else {
		 return 0;
	 }
 }

 //////////////////////////////////////////////////////////////////////////
 // adc functions
 void adc_init(void)
 {
	 // AREF = AVcc
	 ADMUX = (1<<REFS0);
	 
	 // ADC Enable and prescaler of 16
	 // 16000000/128 = 125000
	 ADCSRA = (1<<ADEN)|(1<<ADPS2);
 }

 uint8_t adc_read(uint8_t ch)
 {
	 // select the corresponding channel 0~7
	 // ANDing with ’7? will always keep the value
	 // of ‘ch’ between 0 and 7
	 ch &= 0b00000111;  // AND operation with 7
	 ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	 
	 // start single convertion
	 // write ’1? to ADSC
	 ADCSRA |= (1<<ADSC);
	 
	 // wait for conversion to complete
	 // ADSC becomes ’0? again
	 // till then, run loop continuously
	 while(ADCSRA & (1<<ADSC));
	 
	 return ((ADC) >> 2);
 }

 //////////////////////////////////////////////////////////////////////////
 // send hand data via protocol
 void send_hand_data(void) 
 {
	uint8_t check = PROT_PREAMBLE + PANEL_COUNT + PROT_DATA_HAND;
	UART_send(PROT_PREAMBLE);
	UART_send(PANEL_COUNT);
	UART_send(PROT_DATA_HAND);
	for(uint8_t i = 0; i < PANEL_COUNT; i++){
		uint8_t a_dat = adc_read(i);
		UART_send(a_dat);
		check += a_dat;
	}
	UART_send(~check);
 }

 //////////////////////////////////////////////////////////////////////////
 // send data via protocol
 void Prot_Send(uint8_t* ptr, uint8_t length, uint8_t type)
 {
	 uint8_t check = PROT_PREAMBLE + length + type;
	 UART_send(PROT_PREAMBLE);
	 UART_send(length);
	 UART_send(type);
	 for(uint8_t i = 0; i < length; i++){
		 UART_send(ptr[i]);
		 check += ptr[i];
	 }
	 UART_send(~check);
 }