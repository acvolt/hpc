#include "LiquidCrystal.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#ifndef _BV
#define _BV(bit) (1<<(bit))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { __int16_t t = a; a = b; b = t; }
#endif

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

/*  
Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
	init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
	init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
} */

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t i2caddr) {
	_i2cAddr = i2caddr;

	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	// the I/O expander pinout
	_rs_pin = 1;
	_rw_pin = 255;
	_enable_pin = 2;
	_data_pins[0] = 3;  // really d4
	_data_pins[1] = 4;  // really d5
	_data_pins[2] = 5;  // really d6
	_data_pins[3] = 6;  // really d7

						// we can't begin() yet :(
}



/*  init appears to be for something we're not

void Adafruit_LiquidCrystal::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	_rs_pin = rs;
	_rw_pin = rw;
	_enable_pin = enable;

	_data_pins[0] = d0;
	_data_pins[1] = d1;
	_data_pins[2] = d2;
	_data_pins[3] = d3;
	_data_pins[4] = d4;
	_data_pins[5] = d5;
	_data_pins[6] = d6;
	_data_pins[7] = d7;

	_i2cAddr = 255;
	_SPIclock = _SPIdata = _SPIlatch = 255;

	if (fourbitmode)
		_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	else
		_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
}
*/
void Adafruit_LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	// check if i2c
	if (_i2cAddr != 255) {
		_i2c.begin(_i2cAddr);
		printf("Set Backlight pin as output next command\n");
		_i2c.pinMode(7, OUTPUT); // backlight
		printf("just set the ping mode now setting backlight on \n ");
		_i2c.digitalWrite(7, HIGH); // backlight
		printf("backlight pin should now be high, moving on to set pins 1-4 as outputs");
		for (uint8_t i = 0; i<4; i++)
			_pinMode(_data_pins[i], OUTPUT);

		_i2c.pinMode(_rs_pin, OUTPUT);
		_i2c.pinMode(_enable_pin, OUTPUT);
	}




	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;
	_currline = 0;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	usleep(50000);
	// Now we pull both RS and R/W low to begin commands
	_digitalWrite(_rs_pin, LOW);
	_digitalWrite(_enable_pin, LOW);
	if (_rw_pin != 255) {
		_digitalWrite(_rw_pin, LOW);
	}

	//put the LCD into 4 bit or 8 bit mode
	if (!(_displayfunction & LCD_8BITMODE)) {
		// this is according to the hitachi HD44780 datasheet
		// figure 24, pg 46

		// we start in 8bit mode, try to set 4 bit mode
		write4bits(0x03);
		usleep(4500); // wait min 4.1ms

								 // second try
		write4bits(0x03);
		usleep(4500); // wait min 4.1ms

								 // third go!
		write4bits(0x03);
		usleep(150);

		// finally, set to 8-bit interface
		write4bits(0x02);
	}
	else {
		// this is according to the hitachi HD44780 datasheet
		// page 45 figure 23

		// Send function set command sequence
		command(LCD_FUNCTIONSET | _displayfunction);
		usleep(4500);  // wait more than 4.1ms

								  // second try
		command(LCD_FUNCTIONSET | _displayfunction);
		usleep(150);

		// third go
		command(LCD_FUNCTIONSET | _displayfunction);
	}

	// finally, set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	// clear it off
	clear();

	// Initialize to default text direction (for romance languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

}

/********** high level commands, for the user! */
void Adafruit_LiquidCrystal::clear()
{
	command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	usleep(2000);  // this command takes a long time!
}

void Adafruit_LiquidCrystal::home()
{
	command(LCD_RETURNHOME);  // set cursor position to zero
	usleep(2000);  // this command takes a long time!
}

void Adafruit_LiquidCrystal::setCursor(uint8_t col, uint8_t row)
{
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > _numlines) {
		row = _numlines - 1;    // we count rows starting w/0
	}

	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void Adafruit_LiquidCrystal::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_LiquidCrystal::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void Adafruit_LiquidCrystal::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_LiquidCrystal::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void Adafruit_LiquidCrystal::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_LiquidCrystal::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void Adafruit_LiquidCrystal::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void Adafruit_LiquidCrystal::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void Adafruit_LiquidCrystal::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void Adafruit_LiquidCrystal::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void Adafruit_LiquidCrystal::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void Adafruit_LiquidCrystal::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void Adafruit_LiquidCrystal::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i = 0; i<8; i++) {
		write(charmap[i]);
	}
}

/*********** mid level commands, for sending data/cmds */

inline void Adafruit_LiquidCrystal::command(uint8_t value) {
	send(value, LOW);
}


inline size_t Adafruit_LiquidCrystal::write(uint8_t value) {
	send(value, HIGH);
	return 1;
}


/************ low level data pushing commands **********/

// little wrapper for i/o writes
void  Adafruit_LiquidCrystal::_digitalWrite(uint8_t p, uint8_t d) {
	if (_i2cAddr != 255) {
		// an i2c command
		_i2c.digitalWrite(p, d);
	}
}

// Allows to set the backlight, if the LCD backpack is used
void Adafruit_LiquidCrystal::setBacklight(uint8_t status) {
	// check if i2c or SPI
	if ((_i2cAddr != 255) || (_SPIclock != 255)) {
		_digitalWrite(7, status); // backlight is on pin 7
	}
}

// little wrapper for i/o directions
void  Adafruit_LiquidCrystal::_pinMode(uint8_t p, uint8_t d) {
	if (_i2cAddr != 255) {
		// an i2c command
		_i2c.pinMode(p, d);
	}
}

// write either command or data, with automatic 4/8-bit selection
void Adafruit_LiquidCrystal::send(uint8_t value, bool mode) {
	_digitalWrite(_rs_pin, mode);

	// if there is a RW pin indicated, set it low to Write
	if (_rw_pin != 255) {
		_digitalWrite(_rw_pin, LOW);
	}

	if (_displayfunction & LCD_8BITMODE) {
		write8bits(value);
	}
	else {
		write4bits(value >> 4);
		write4bits(value);
	}
}

void Adafruit_LiquidCrystal::pulseEnable(void) {
	_digitalWrite(_enable_pin, LOW);
	usleep(1);
	_digitalWrite(_enable_pin, HIGH);
	usleep(1);    // enable pulse must be >450ns
	_digitalWrite(_enable_pin, LOW);
	usleep(100);   // commands need > 37us to settle
}

void Adafruit_LiquidCrystal::write4bits(uint8_t value) {
	if (_i2cAddr != 255) {
		uint8_t out = 0;

		out = _i2c.readGPIO();


		// speed up for i2c since its sluggish
		for (int i = 0; i < 4; i++) {
			out &= ~_BV(_data_pins[i]);
			out |= ((value >> i) & 0x1) << _data_pins[i];
		}

		// make sure enable is low
		out &= ~_BV(_enable_pin);

		_i2c.writeGPIO(out);

		// pulse enable
		usleep(1);
		out |= _BV(_enable_pin);
		_i2c.writeGPIO(out);
		usleep(1);
		out &= ~_BV(_enable_pin);
		_i2c.writeGPIO(out);
		usleep(100);
	}
	else {
		for (int i = 0; i < 4; i++) {
			_pinMode(_data_pins[i], OUTPUT);
			_digitalWrite(_data_pins[i], (value >> i) & 0x01);
		}
		pulseEnable();
	}
}

void Adafruit_LiquidCrystal::write8bits(uint8_t value) {
	for (int i = 0; i < 8; i++) {
		_pinMode(_data_pins[i], OUTPUT);
		_digitalWrite(_data_pins[i], (value >> i) & 0x01);
	}

	pulseEnable();
}


 size_t Print::write(const uint8_t *buffer, size_t size)
{
	size_t n = 0;
	while (size--) {
		if (write(*buffer++)) n++;
		else break;
	}
	return n;
}

//size_t Print::print(const __FlashStringHelper *ifsh)
//{
//	PGM_P p = reinterpret_cast<PGM_P>(ifsh);
//	size_t n = 0;
//	while (1) {
//		unsigned char c = pgm_read_byte(p++);
//		if (c == 0) break;
//		if (write(c)) n++;
//		else break;
//	}
//	return n;
//}

size_t Print::print(const string &s)
{
	return write(s.c_str(), s.length());
}

size_t Print::print(const char str[])
{
	return write(str);
}

size_t Print::print(char c)
{
	return write(c);
}

size_t Print::print(unsigned char b, int base)
{
	return print((unsigned long)b, base);
}

size_t Print::print(int n, int base)
{
	return print((long)n, base);
}

size_t Print::print(unsigned int n, int base)
{
	return print((unsigned long)n, base);
}

size_t Print::print(long n, int base)
{
	if (base == 0) {
		return write(n);
	}
	else if (base == 10) {
		if (n < 0) {
			int t = print('-');
			n = -n;
			return printNumber(n, 10) + t;
		}
		return printNumber(n, 10);
	}
	else {
		return printNumber(n, base);
	}
}

size_t Print::print(unsigned long n, int base)
{
	if (base == 0) return write(n);
	else return printNumber(n, base);
}

size_t Print::print(double n, int digits)
{
	return printFloat(n, digits);
}

//size_t Print::println(const __FlashStringHelper *ifsh)
//{
//	size_t n = print(ifsh);
//	n += println();
//	return n;
//}

//size_t Print::print(const Printable& x)
//{
//	return x.printTo(*this);
//}

size_t Print::println(void)
{
	return write("\r\n");
}

size_t Print::println(const string &s)
{
	size_t n = print(s);
	n += println();
	return n;
}

size_t Print::println(const char c[])
{
	size_t n = print(c);
	n += println();
	return n;
}

size_t Print::println(char c)
{
	size_t n = print(c);
	n += println();
	return n;
}

size_t Print::println(unsigned char b, int base)
{
	size_t n = print(b, base);
	n += println();
	return n;
}

size_t Print::println(int num, int base)
{
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(unsigned int num, int base)
{
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(long num, int base)
{
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(unsigned long num, int base)
{
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(double num, int digits)
{
	size_t n = print(num, digits);
	n += println();
	return n;
}

//size_t Print::println(const Printable& x)
//{
//	size_t n = print(x);
//	n += println();
//	return n;
//}

// Private Methods /////////////////////////////////////////////////////////////

size_t Print::printNumber(unsigned long n, uint8_t base)
{
	char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
	char *str = &buf[sizeof(buf) - 1];

	*str = '\0';

	// prevent crash if called with base == 1
	if (base < 2) base = 10;

	do {
		char c = n % base;
		n /= base;

		*--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while (n);

	return write(str);
}

size_t Print::printFloat(double number, uint8_t digits)
{
	size_t n = 0;

	if (isnan(number)) return print("nan");
	if (isinf(number)) return print("inf");
	if (number > 4294967040.0) return print("ovf");  // constant determined empirically
	if (number <-4294967040.0) return print("ovf");  // constant determined empirically

													 // Handle negative numbers
	if (number < 0.0)
	{
		n += print('-');
		number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i = 0; i<digits; ++i)
		rounding /= 10.0;

	number += rounding;

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	n += print(int_part);

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0) {
		n += print(".");
	}

	// Extract digits from the remainder one at a time
	while (digits-- > 0)
	{
		remainder *= 10.0;
		unsigned int toPrint = (unsigned int)(remainder);
		n += print(toPrint);
		remainder -= toPrint;
	}

	return n;
}



