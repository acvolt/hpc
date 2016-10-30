#pragma once
#ifndef LEDBACKPACK_H_
#define LEDBACKPACK_H_

#include <unistd.h>

#define LED_ON 1
#define LED_OFF 0

#define LED_RED 1
#define LED_YELLOW 2
#define LED_GREEN 3



#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0

#define SEVENSEG_DIGITS 5

class Smith_LEDBackpack {
public:
	Smith_LEDBackpack(void);
	void begin(__uint32_t _addr, int fd);
	void setBrightness(__uint8_t b);
	void blinkRate(__uint8_t b);
	void writeDisplay(void);
	void clear(void);

	__uint16_t displaybuffer[8];

	void init(__uint32_t a);
protected:
	__uint32_t i2c_file;
	__uint32_t i2c_addr;
};

class Smith_AlphaNum4 : public Smith_LEDBackpack {
public:
	Smith_AlphaNum4(void);

	void writeDigitRaw(__uint8_t n, __uint16_t bitmask);
	void writeDigitAscii(__uint8_t n, __uint8_t ascii, bool dot = false);

private:


};


#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0

class Smith_7segment : public Smith_LEDBackpack{
public:
	Smith_7segment(void);
	size_t writer(__uint8_t c);

	void print(char, int = BYTE);
	void print(unsigned char, int = BYTE);
	void print(int, int = DEC);
	void print(unsigned int, int = DEC);
	void print(long, int = DEC);
	void print(unsigned long, int = DEC);
	void print(double, int = 2);
	void println(char, int = BYTE);
	void println(unsigned char, int = BYTE);
	void println(int, int = DEC);
	void println(unsigned int, int = DEC);
	void println(long, int = DEC);
	void println(unsigned long, int = DEC);
	void println(double, int = 2);
	void println(void);

	void writeDigitRaw(__uint8_t x, __uint8_t bitmask);
	void writeDigitNum(__uint8_t x, __uint8_t num, bool dot = false);
	void drawColon(bool state);
	void printNumber(long, __uint8_t = 2);
	void printFloat(double, __uint8_t = 2, __uint8_t = DEC);
	void printError(void);

	void writeColon(void);

private:
	__uint8_t position;
};






#endif /* LEDBACKPACK_H_ */
