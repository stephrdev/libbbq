#include <avr/io.h>
#include "serial.h"


Serial::Serial(uint16_t speed) {
	uint16_t baud = (F_CPU / 4 / speed - 1) / 2;

	// Do what usually happens in avr-libc setbaud.h
	if (((F_CPU == 16000000UL) && (baud == 57600)) || (baud > 4095)) {
		UCSR0A &= ~(1 << U2X0);
		baud = (F_CPU / 8 / speed - 1) / 2;
	} else {
		UCSR0A |= (1 << U2X0);
	}

	// Split baud to fit in two registers
	UBRR0H = baud >> 8;
	UBRR0L = baud;

	// Enable transmitter and receiver
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	// Configure 8N1
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void Serial::transmit_byte(uint8_t input) {
	// Wait for an empty transmit buffer
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = input;
}

uint8_t Serial::receive_byte(void) {
	// Data available, data available, data available?
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

uint8_t Serial::has_data(void) {
	return bit_is_set(UCSR0A, RXC0);
}

uint8_t Serial::is_ready(void) {
	return bit_is_set(UCSR0A, UDRE0);
}

void Serial::transmit_number(int8_t input) {
	if (input < 0) {
		transmit_byte('-');
		transmit_number((uint8_t)(input * -1));
	} else {
		transmit_byte('+');
		transmit_number((uint8_t)(input));
	}
}

void Serial::transmit_number(uint8_t input) {
	transmit_byte('0' + (input / 100));
	transmit_byte('0' + ((input / 10) % 10));
	transmit_byte('0' + (input % 10));
}

void Serial::transmit_text(const char input[]) {
	uint8_t i = 0;
	while (input[i]) {
		transmit_byte(input[i]);
		i++;
	}
}

void Serial::transmit_text(const char input[], bool newline) {
	transmit_text(input);

	if (newline) {
		// Add newline and return chars.
		transmit_byte('\r');
		transmit_byte('\n');
	}
}

uint8_t Serial::receive_text(char output[], uint8_t max_length) {
	uint8_t received, i;

	for(i = 0; i < max_length; i++) {
		// Get byte and echo back.
		received = receive_byte();
		transmit_byte(received);

		if (received == '\r') {
			// End of line. We're done.
			break;
		} else {
			output[i] = received;
			i++;
		}
	}

	// Add NULL to the end.
	output[i] = '\0';
	return i;
}
