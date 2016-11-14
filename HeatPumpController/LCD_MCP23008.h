#pragma once


#ifndef _Smith_LCD_MCP23008_H_
#define _Smith_LCD_MCP23008_H_

#include <inttypes.h>
#include "Smith_I2C.h"


#define INPUT 0
#define OUTPUT 1
#define CHANGE 1
#define FALLING 2
#ifndef HIGH
#define HIGH 0x01
#endif

//For compatibility from Arduino Library
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

class LCD_MCP23008 {
public:
//	MCP23008() { _i2cdev = Smith_I2C(0x24); }
	void begin(uint8_t addr);
	void begin(void);

	void pinMode(uint8_t p, uint8_t d);
	void digitalWrite(uint8_t p, uint8_t d);
	void pullUp(uint8_t p, uint8_t d);
	uint8_t digitalRead(uint8_t p);


	void writeGPIO(uint8_t value);
	uint8_t readGPIO(void);

	void setupInterrupts(uint8_t mirroring, uint8_t open, uint8_t polarity);
	void setupInterruptPin(uint8_t p, uint8_t mode);
	uint8_t getLastInterruptPin();
	uint8_t getLastInterruptPinValue();


private:
	uint8_t i2caddr;
	

	uint8_t bitForPin(uint8_t pin);
	uint8_t regForPin(uint8_t pin, uint8_t portAddr);

	uint8_t readRegister(uint8_t addr);
	void writeRegister(uint8_t addr, uint8_t value);

	/**
	* Utility private method to update a register associated with a pin (whether port A/B)
	* reads its value, updates the particular bit, and writes its value.
	*/
	void updateRegisterBit(uint8_t p, uint8_t pValue, uint8_t portAddr);

protected:
//	Smith_I2C _i2cdev;
};

#define MCP23008_ADDRESS 0x20

// registers
#define MCP23008_IODIR 0x00
#define MCP23008_IPOL 0x01
#define MCP23008_GPINTEN 0x02
#define MCP23008_DEFVAL 0x03
#define MCP23008_INTCON 0x04
#define MCP23008_IOCON 0x05
#define MCP23008_GPPU 0x06
#define MCP23008_INTF 0x07
#define MCP23008_INTCAP 0x08
#define MCP23008_GPIO 0x09
#define MCP23008_OLAT 0x0A




#define MCP23008_INT_ERR 255

#endif
