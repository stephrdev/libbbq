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

void Serial::transmitByte(uint8_t input) {
	// Wait for an empty transmit buffer
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = input;
}

uint8_t Serial::receiveByte(void) {
	// Data available, data available, data available?
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

uint8_t Serial::hasData(void) {
	return bit_is_set(UCSR0A, RXC0);
}

uint8_t Serial::isReady(void) {
	return bit_is_set(UCSR0A, UDRE0);
}

void Serial::print(const char input[]) {
	uint8_t i = 0;
	while (input[i]) {
		transmitByte(input[i]);
		i++;
	}
}

void Serial::printLine(const char input[]) {
	print(input);
	// Add newline and return chars.
	transmitByte('\r');
	transmitByte('\n');
}

uint8_t Serial::readLine(char line[], uint8_t maxLength) {
	uint8_t received, i;

	for(i = 0; i < maxLength; i++) {
		// Get byte and echo back.
		received = receiveByte();
		transmitByte(received);

		if (received == '\r') {
			// End of line. We're done.
			break;
		} else {
			line[i] = received;
			i++;
		}
	}

	// Add NULL to the end.
	line[i] = '\0';
	return i;
}
