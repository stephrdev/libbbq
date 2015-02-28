#include <stdint.h>


class Serial {
	public:
		Serial(uint16_t speed);

		void transmitByte(uint8_t input);
		uint8_t receiveByte(void);

		uint8_t hasData(void);
		uint8_t isReady(void);

		void print(const char input[]);
		void printNumber(uint8_t intput);
		void printLine(const char input[]);
		uint8_t readLine(char line[], uint8_t maxLength);
};
