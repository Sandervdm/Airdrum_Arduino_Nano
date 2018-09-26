/*
 * Quest Arduino.c
 *
 * Created: 24-4-2017 11:06:56
 * Author : Sander
 */ 

 #include "main.h"

 RGB_Data LEDS[PANEL_COUNT];

int main(void)
{
	//initialize leds
	LED_DDR |= LED_PIN;
	StripShow();

	//initialize adc for hand detection
	adc_init();

	//initialize uart on 9600 baud
	UART_init();

	//start timer
	SetTimer1Delay33ms();

	// Zero the LED buffer
	memset(LEDS, 0, sizeof(RGB_Data)*PANEL_COUNT);

    while (1)
    {
		// If there is UART data available, handle it
		if(UART_HAS_DATA())
		{
			// Check the startbyte
			if(UART_get() == 0x55) {
				
				// read the datalength
				uint8_t data_length = UART_get();
				// Create a buffer for the data
				uint8_t data[data_length];
				// Receive the data type
				uint8_t data_type = UART_get();
				// Create check value 
				uint8_t check_val_r = 0x55 + data_length + data_type;
				// Receive all data
				for(uint8_t i = 0; i < data_length; i++) {
					data[i] = UART_get();
					check_val_r += data[i];
				}
				// Receive the check value
				uint8_t check_val = UART_get();

				// Check the check value
				if(check_val == check_val_r) {

					// switch the received datatype
					if(data_type == PROT_DATA_RGB && data_length == 20) {
						// Update LED data
						setColor(LEDS, data);
						uint8_t ackval = PROT_ACK;
						Prot_Send(&ackval, 1, PROT_ACK);
					}
					else if(data_type == PROT_NACK) {
						// last data didn't come through, send again
						send_hand_data();
					}
				}
				else {
					// Checksum error, sent NACK
					uint8_t nack_val = PROT_NACK;
					Prot_Send(&nack_val, 1, PROT_NACK);
				}
			}
		}

		// Every 33ms sent hand data and update LEDs
		if(Timer1Empty())
		{
			// Send analog data to rpi
			send_hand_data();
			// Refresh LED buffer
			updateLeds(LEDS);
			// Show new data on strip
			StripShow();
			// Set timer again
			SetTimer1Delay33ms();
		}
    }
}

