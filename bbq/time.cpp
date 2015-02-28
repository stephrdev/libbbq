#include <avr/io.h>
#include <util/atomic.h>

#include "time.h"


volatile uint16_t milliseconds;


ISR(TIMER0_COMPA_vect){
	++milliseconds;
}

void time_init(void) {
	TCCR0A |= (1 << WGM01); // CTC Mode

	// Set clock select (TCCR0B) and compare value (OCR0A)
	if (F_CPU > 16320000) {
		// 16.32MHz - 65.28MHz
		TCCR0B |= (1 << CS20);
		OCR0A = ((F_CPU / 256) / 1000);
	} else if (F_CPU > 2040000) {
		// 2.04MHz - 16.32MHz
		TCCR0B |= ((1 << CS00) | (1 << CS01));
		OCR0A = ((F_CPU / 64) / 1000);
	} else if (F_CPU > 255) {
		 // 256Hz - 2.04MHz
		TCCR0B |= (1 << CS01);
		OCR0A = (uint8_t)((F_CPU / 8) / 1000);
	}

	TIMSK0 = (1 << OCIE0A); // Enable compare interrupt.
}

// Get current milliseconds
uint16_t time_ms() {
	uint16_t ms;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		ms = milliseconds;
	}
	return ms;
}
