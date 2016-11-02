
#include <iostream>
#include <wiringPiI2C.h>
#include <stdio.h>
//#include <sys/ioctl.h>
//#include <linux/i2c-dev.h>
#include "Smith_I2C.h"




#include "LEDBackpack.h"



using namespace std;

#ifndef _BV
#define _BV(bit) (1<<(bit))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { __int16_t t = a; a = b; b = t; }
#endif

Smith_I2C i2cDev = Smith_I2C();

static const __uint8_t numbertable[] = {
	0x3F, /* 0 */
	0x06, /* 1 */
	0x5B, /* 2 */
	0x4F, /* 3 */
	0x66, /* 4 */
	0x6D, /* 5 */
	0x7D, /* 6 */
	0x07, /* 7 */
	0x7F, /* 8 */
	0x6F, /* 9 */
	0x77, /* a */
	0x7C, /* b */
	0x39, /* C */
	0x5E, /* d */
	0x79, /* E */
	0x71, /* F */
};

static const __uint16_t alphafonttable[] {

	0b0000000000000001,
	0b0000000000000010,
	0b0000000000000100,
	0b0000000000001000,
	0b0000000000010000,
	0b0000000000100000,
	0b0000000001000000,
	0b0000000010000000,
	0b0000000100000000,
	0b0000001000000000,
	0b0000010000000000,
	0b0000100000000000,
	0b0001000000000000,
	0b0010000000000000,
	0b0100000000000000,
	0b1000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0001001011001001,
	0b0001010111000000,
	0b0001001011111001,
	0b0000000011100011,
	0b0000010100110000,
	0b0001001011001000,
	0b0011101000000000,
	0b0001011100000000,
	0b0000000000000000, // 
	0b0000000000000110, // !
	0b0000001000100000, // "
	0b0001001011001110, // #
	0b0001001011101101, // $
	0b0000110000100100, // %
	0b0010001101011101, // &
	0b0000010000000000, // '
	0b0010010000000000, // (
	0b0000100100000000, // )
	0b0011111111000000, // *
	0b0001001011000000, // +
	0b0000100000000000, // ,
	0b0000000011000000, // -
	0b0000000000000000, // .
	0b0000110000000000, // /
	0b0000110000111111, // 0
	0b0000000000000110, // 1
	0b0000000011011011, // 2
	0b0000000010001111, // 3
	0b0000000011100110, // 4
	0b0010000001101001, // 5
	0b0000000011111101, // 6
	0b0000000000000111, // 7
	0b0000000011111111, // 8
	0b0000000011101111, // 9
	0b0001001000000000, // :
	0b0000101000000000, // ;
	0b0010010000000000, // <
	0b0000000011001000, // =
	0b0000100100000000, // >
	0b0001000010000011, // ?
	0b0000001010111011, // @
	0b0000000011110111, // A
	0b0001001010001111, // B
	0b0000000000111001, // C
	0b0001001000001111, // D
	0b0000000011111001, // E
	0b0000000001110001, // F
	0b0000000010111101, // G
	0b0000000011110110, // H
	0b0001001000000000, // I
	0b0000000000011110, // J
	0b0010010001110000, // K
	0b0000000000111000, // L
	0b0000010100110110, // M
	0b0010000100110110, // N
	0b0000000000111111, // O
	0b0000000011110011, // P
	0b0010000000111111, // Q
	0b0010000011110011, // R
	0b0000000011101101, // S
	0b0001001000000001, // T
	0b0000000000111110, // U
	0b0000110000110000, // V
	0b0010100000110110, // W
	0b0010110100000000, // X
	0b0001010100000000, // Y
	0b0000110000001001, // Z
	0b0000000000111001, // [
	0b0010000100000000, //
	0b0000000000001111, // ]
	0b0000110000000011, // ^
	0b0000000000001000, // _
	0b0000000100000000, // `
	0b0001000001011000, // a
	0b0010000001111000, // b
	0b0000000011011000, // c
	0b0000100010001110, // d
	0b0000100001011000, // e
	0b0000000001110001, // f
	0b0000010010001110, // g
	0b0001000001110000, // h
	0b0001000000000000, // i
	0b0000000000001110, // j
	0b0011011000000000, // k
	0b0000000000110000, // l
	0b0001000011010100, // m
	0b0001000001010000, // n
	0b0000000011011100, // o
	0b0000000101110000, // p
	0b0000010010000110, // q
	0b0000000001010000, // r
	0b0010000010001000, // s
	0b0000000001111000, // t
	0b0000000000011100, // u
	0b0010000000000100, // v
	0b0010100000010100, // w
	0b0010100011000000, // x
	0b0010000000001100, // y
	0b0000100001001000, // z
	0b0000100101001001, // {
	0b0001001000000000, // |
	0b0010010010001001, // }
	0b0000010100100000, // ~
	0b0011111111111111,

};

void Smith_LEDBackpack::setBrightness(__uint8_t b) {
	char buffer[1];
	if (b > 15) b = 15;

	//wiringPiI2CWrite(i2c_addr, HT16K33_CMD_BRIGHTNESS | b);
	buffer[0] = HT16K33_CMD_BRIGHTNESS | b;
	
	/* if (ioctl(i2c_file, I2C_SLAVE, i2c_addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	if (write(i2c_file, buffer, 1) != 1) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	*/
	//i2cDev.i2cWriteBlock(i2c_addr, buffer, 1);
	i2cDev.i2cWriteByte(i2c_addr, buffer[0]);
}

void Smith_LEDBackpack::blinkRate(__uint8_t b) {
	char buffer[1];
	if (b > 3) b = 0; // turn off if not sure
//	wiringPiI2CWrite(i2c_addr, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1));

	buffer[0] = HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1);
	/*
	if (ioctl(i2c_file, I2C_SLAVE, i2c_addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	if (write(i2c_file, buffer, 1) != 1) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	} */
	i2cDev.i2cWriteByte(i2c_addr, buffer[0]);
	//i2cDev.i2cWriteBlock(i2c_addr, buffer, 1);
}

Smith_LEDBackpack::Smith_LEDBackpack(void) {
}

void Smith_LEDBackpack::begin(__uint32_t _addr = 0x70, int _fd = 0) {
	i2c_addr = _addr;
	i2c_file = _fd;
//	Smith_I2C i2cDev = Smith_I2C(_addr);
	i2cDev.i2cSetup(_addr);

	char buffer[1] = { 0x21 };
	i2cDev.i2cWriteByte(i2c_addr, buffer[0]);
	/*
	if (ioctl(i2c_file, I2C_SLAVE, i2c_addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	if (write(i2c_file, buffer, 1) != 1) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	} */
//	i2cDev.i2cWriteByte(0x70, 0);
	//i2cDev.i2cWriteBlock(i2c_addr, buffer, 1);

	//  Wire.begin();

	//  Wire.beginTransmission(i2c_addr);
	//  Wire.write(0x21);  // turn on oscillator
	//  Wire.endTransmission();
//	wiringPiI2CWrite(i2c_addr, 0);
	blinkRate(HT16K33_BLINK_OFF);

	setBrightness(15); // max brightness
}

void Smith_LEDBackpack::writeDisplay(void) {

	int length = 8;
	unsigned char buffer[1] = { 0x0 };

	/*
	__uint8_t newBuffer[16];

	void* newArray;
	newArray = displaybuffer;

//	for (int i = 0; i < 16; i++)
//	{
		newBuffer[1] = *static_cast<__uint8_t*>(newArray);
		newArray+= 1;
		newBuffer[0] = *static_cast<__uint8_t*>(newArray);
		newArray+=1;
		newBuffer[3] = *static_cast<__uint8_t*>(newArray);
		newArray+=1;
		newBuffer[2] = *static_cast<__uint8_t*>(newArray);
		newArray+=1;
		newBuffer[5] = *static_cast<__uint8_t*>(newArray);
		newArray+=1;
		newBuffer[4] = *static_cast<__uint8_t*>(newArray);
		newArray+=1;
		newBuffer[7] = *static_cast<__uint8_t*>(newArray);
		newArray+=1;
		newBuffer[6] = *static_cast<__uint8_t*>(newArray);
		newArray+=1;
		newBuffer[9] = *static_cast<__uint8_t*>(newArray);
		newArray += 1;
		newBuffer[8] = *static_cast<__uint8_t*>(newArray);
		newArray += 1;
		newBuffer[11] = *static_cast<__uint8_t*>(newArray);
		newArray += 1;
		newBuffer[10] = *static_cast<__uint8_t*>(newArray);
		newArray += 1;
		newBuffer[13] = *static_cast<__uint8_t*>(newArray);
		newArray += 1;
		newBuffer[12] = *static_cast<__uint8_t*>(newArray);
		newArray += 1;
		newBuffer[15] = *static_cast<__uint8_t*>(newArray);
		newArray += 1;
		newBuffer[14] = *static_cast<__uint8_t*>(newArray);


//	} */
		/*
	if (ioctl(i2c_file, I2C_SLAVE, i2c_addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	cout << "B0:" << displaybuffer[0] << " B1:" << displaybuffer[1] << " B2:" << displaybuffer[2] << " B3" << displaybuffer[3] << " B4:" << displaybuffer[4] << " B5:" << displaybuffer[5] << " B6:" << displaybuffer[6] << " B7:" << displaybuffer[7] << endl;
	cout << "B0:" << newBuffer[0] << " B1:" << newBuffer[1] << " B2:" << newBuffer[2] << " B3" << newBuffer[3] << " B4:" << newBuffer[4] << " B5:" << newBuffer[5] << " B6:" << newBuffer[6] << " B7:" << newBuffer[7] << endl;
	if (write(i2c_file, buffer, 1) != 1) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	length = 16;
	if (write(i2c_file, newBuffer, length) != length) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}*/

		length = 16;
		i2cDev.i2cWriteBlock(i2c_addr, buffer, displaybuffer, length);

}

void Smith_LEDBackpack::clear(void) {
	for (__uint8_t i = 0; i<8; i++) {
		displaybuffer[i] = 0;
	}
}

/******************************* QUAD ALPHANUM OBJECT */

Smith_AlphaNum4::Smith_AlphaNum4(void) {

}

void Smith_AlphaNum4::writeDigitRaw(__uint8_t n, __uint16_t bitmask) {
	displaybuffer[n] = bitmask;
}

void Smith_AlphaNum4::writeDigitAscii(__uint8_t n, __uint8_t a, bool d) {
	__uint16_t font = alphafonttable[a];

//	cout << "Digit Lookup Position" << n << " Character " << a << endl;

	displaybuffer[n] = font;

	/*
	Serial.print(a, DEC);
	Serial.print(" / '"); Serial.write(a);
	Serial.print("' = 0x"); Serial.println(font, HEX);
	*/

	if (d) displaybuffer[n] |= (1 << 14);
}


/******************************* 7 SEGMENT OBJECT */

Smith_7segment::Smith_7segment(void) {
	position = 0;
}

void Smith_7segment::print(unsigned long n, int base)
{
//	cout << "in LedBackpack print u long " << n << base << endl;
	if (base == 0) writer(n);
	else printNumber(n, base);
}

void Smith_7segment::print(char c, int base)
{
	print((long)c, base);
}

void Smith_7segment::print(unsigned char b, int base)
{
	print((unsigned long)b, base);
}

void Smith_7segment::print(int n, int base)
{

//	cout << "in LedBackpack N:" << n << " Base: " << base << endl;
	print((long)n, base);
}

void Smith_7segment::print(unsigned int n, int base)
{
	print((unsigned long)n, base);
}

void  Smith_7segment::println(void) {
	position = 0;
}

void  Smith_7segment::println(char c, int base)
{
	print(c, base);
	println();
}

void  Smith_7segment::println(unsigned char b, int base)
{
	print(b, base);
	println();
}

void  Smith_7segment::println(int n, int base)
{

	print(n, base);
	println();
}

void  Smith_7segment::println(unsigned int n, int base)
{
	print(n, base);
	println();
}

void  Smith_7segment::println(long n, int base)
{
	print(n, base);
	println();
}

void  Smith_7segment::println(unsigned long n, int base)
{
	print(n, base);
	println();
}

void  Smith_7segment::println(double n, int digits)
{
	print(n, digits);
	println();
}

void  Smith_7segment::print(double n, int digits)
{
	printFloat(n, digits);
}


size_t Smith_7segment::writer(__uint8_t c) {

	__uint8_t r = 0;
	

	if (c == '\n') position = 0;
	if (c == '\r') position = 0;

	if ((c >= '0') && (c <= '9')) {
		writeDigitNum(position, c - '0');
		r = 1;
	}

	position++;
	if (position == 2) position++;

//	cout << "in LedBackpack writer " << c << " r " << r << endl;

	return r;
}

void Smith_7segment::writeDigitRaw(__uint8_t d, __uint8_t bitmask) {
	if (d > 4) return;
	displaybuffer[d] = bitmask;
}

void Smith_7segment::drawColon(bool state) {
	if (state)
		displaybuffer[2] = 0x2;
	else
		displaybuffer[2] = 0;
}

void Smith_7segment::writeColon(void) {
	//    Wire.beginTransmission(i2c_addr);
	//    Wire.write((uint8_t)0x04); // start at address $02

	//    Wire.write(displaybuffer[2] & 0xFF);
	//    Wire.write(displaybuffer[2] >> 8);

	//    Wire.endTransmission();

	 unsigned char buffer[] = { 0x04 };
	/*
	if (ioctl(i2c_file, I2C_SLAVE, i2c_addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	if (write(i2c_file, buffer, 1) != 1) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	if (write(i2c_file, displaybuffer+32, 2) != 1) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	*/
	i2cDev.i2cWriteBlock(i2c_addr, buffer, displaybuffer+32, 2);
}

void Smith_7segment::writeDigitNum(__uint8_t d, __uint8_t num, bool dot) {
	if (d > 4) return;
//	cout << "in writeDigitNum " << num << " digit " << d << endl;
	writeDigitRaw(d, numbertable[num] | (dot << 7));
}

void Smith_7segment::print(long n, int base)
{
	printNumber(n, base);
}

void Smith_7segment::printNumber(long n, __uint8_t base)
{
	printFloat(n, 0, base);
}

void Smith_7segment::printFloat(double n, __uint8_t fracDigits, __uint8_t base)
{
	__uint8_t numericDigits = 4;   // available digits on display
	bool isNegative = false;  // true if the number is negative

								 // is the number negative?
	if (n < 0) {
		isNegative = true;  // need to draw sign later
		--numericDigits;    // the sign will take up one digit
		n *= -1;            // pretend the number is positive
	}

	// calculate the factor required to shift all fractional digits
	// into the integer part of the number
	double toIntFactor = 1.0;
	for (int i = 0; i < fracDigits; ++i) toIntFactor *= base;

	// create integer containing digits to display by applying
	// shifting factor and rounding adjustment
	__uint32_t displayNumber = n * toIntFactor + 0.5;

	// calculate upper bound on displayNumber given
	// available digits on display
	__uint32_t tooBig = 1;
	for (int i = 0; i < numericDigits; ++i) tooBig *= base;

	// if displayNumber is too large, try fewer fractional digits
	while (displayNumber >= tooBig) {
		--fracDigits;
		toIntFactor /= base;
		displayNumber = n * toIntFactor + 0.5;
	}

	// did toIntFactor shift the decimal off the display?
	if (toIntFactor < 1) {
		printError();
	}
	else {
		// otherwise, display the number
		__int8_t displayPos = 4;

		if (displayNumber)  //if displayNumber is not 0
		{
			for (__uint8_t i = 0; displayNumber || i <= fracDigits; ++i) {
				bool displayDecimal = (fracDigits != 0 && i == fracDigits);
				writeDigitNum(displayPos--, displayNumber % base, displayDecimal);
				if (displayPos == 2) writeDigitRaw(displayPos--, 0x00);
				displayNumber /= base;
			}
		}
		else {
			writeDigitNum(displayPos--, 0, false);
		}

		// display negative sign if negative
		if (isNegative) writeDigitRaw(displayPos--, 0x40);

		// clear remaining display positions
		while (displayPos >= 0) writeDigitRaw(displayPos--, 0x00);
	}
}

void Smith_7segment::printError(void) {
	for (__uint8_t i = 0; i < SEVENSEG_DIGITS; ++i) {
		writeDigitRaw(i, (i == 2 ? 0x00 : 0x40));
	}
}




