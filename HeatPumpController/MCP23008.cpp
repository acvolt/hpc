

#include "MCP23008.h"


Smith_I2C _i2cdev = Smith_I2C();


/**
* Bit number associated to a give Pin
*/
uint8_t MCP23008::bitForPin(uint8_t pin) {
	return pin % 8;
}

/**
* Register address, port dependent, for a given PIN
*/
uint8_t MCP23008::regForPin(uint8_t pin, uint8_t portAddr) {
	return portAddr;
}

/**
* Reads a given register
*/
uint8_t MCP23008::readRegister(uint8_t addr) {
	// read the current GPINTEN
	/*
	Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
	wiresend(addr);
	Wire.endTransmission();
	Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
	return wirerecv();

	*/
	uint8_t buffer[1];

	_i2cdev.i2cReadBlock(i2caddr, &addr, buffer, 1);

	return buffer[1];
}


/**
* Writes a given register
*/
void MCP23008::writeRegister(uint8_t regAddr, uint8_t regValue) {
	// Write the register
	/*
	Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
	wiresend(regAddr);
	wiresend(regValue);
	Wire.endTransmission();
	*/

	_i2cdev.i2cWriteBlock(i2caddr, &regAddr, &regValue, 1);

}


/**
* Helper to update a single bit of an A/B register.
* - Reads the current register value
* - Writes the new register value
*/
void MCP23008::updateRegisterBit(uint8_t pin, uint8_t pValue, uint8_t portAaddr) {
	uint8_t regValue;
	uint8_t regAddr = regForPin(pin, portAaddr);
	uint8_t bit = bitForPin(pin);
	regValue = readRegister(regAddr);

	// set the value for the particular bit
	//bitWrite(regValue, bit, pValue);

	regValue = regValue & ((pValue & 1) << bit);

	writeRegister(regAddr, regValue);
}

////////////////////////////////////////////////////////////////////////////////

/**
* Initializes the MCP23017 given its HW selected address, see datasheet for Address selection.
*/
void MCP23008::begin(uint8_t addr) {
	if (addr > 7) {
		addr = 7;
	}
	i2caddr = addr;

	_i2cdev.i2cSetup();

	// set defaults!
	// all inputs on port A and B
	writeRegister(MCP23008_IODIR, 0xff);
}

/**
* Initializes the default MCP23017, with 000 for the configurable part of the address
*/
void MCP23008::begin(void) {
	begin(0);
}

/**
* Sets the pin mode to either INPUT or OUTPUT
*/
void MCP23008::pinMode(uint8_t p, uint8_t d) {
	updateRegisterBit(p, (d == INPUT), MCP23008_IODIR);
}



/**
* Read a single port, A or B, and return its current 8 bit value.
* Parameter b should be 0 for GPIOA, and 1 for GPIOB.
*/
uint8_t MCP23008::readGPIO(void) {

	// read the current GPIO output latches
/*	Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);

	wiresend(MCP23008_GPIO);

	Wire.endTransmission();

	Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
	return wirerecv();
	*/
	uint8_t reg[] = { MCP23008_GPIO };

	
}

/**
* Writes all the pins in one go. This method is very useful if you are implementing a multiplexed matrix and want to get a decent refresh rate.
*/


void MCP23008::digitalWrite(uint8_t pin, uint8_t d) {
	uint8_t gpio;
	uint8_t bit = bitForPin(pin);


	// read the current GPIO output latches
	uint8_t regAddr = regForPin(pin, MCP23008_OLAT);
	gpio = readRegister(regAddr);

	// set the pin and direction
	bitWrite(gpio, bit, d);

	// write the new GPIO
	regAddr = regForPin(pin, MCP23008_GPIO);
	writeRegister(regAddr, gpio);
}

void MCP23008::pullUp(uint8_t p, uint8_t d) {
	updateRegisterBit(p, d, MCP23008_GPPU);
}

uint8_t MCP23008::digitalRead(uint8_t pin) {
	uint8_t bit = bitForPin(pin);
	uint8_t regAddr = regForPin(pin, MCP23008_GPIO);
	return (readRegister(regAddr) >> bit) & 0x1;
}

/**
* Configures the interrupt system. both port A and B are assigned the same configuration.
* Mirroring will OR both INTA and INTB pins.
* Opendrain will set the INT pin to value or open drain.
* polarity will set LOW or HIGH on interrupt.
* Default values after Power On Reset are: (false,flase, LOW)
* If you are connecting the INTA/B pin to arduino 2/3, you should configure the interupt handling as FALLING with
* the default configuration.
*/
void MCP23008::setupInterrupts(uint8_t mirroring, uint8_t openDrain, uint8_t polarity) {
	// configure the port A
	uint8_t ioconfValue = readRegister(MCP23008_IOCON);
	bitWrite(ioconfValue, 2, openDrain);
	bitWrite(ioconfValue, 1, polarity);
	writeRegister(MCP23008_IOCON, ioconfValue);

}

/**
* Set's up a pin for interrupt. uses arduino MODEs: CHANGE, FALLING, RISING.
*
* Note that the interrupt condition finishes when you read the information about the port / value
* that caused the interrupt or you read the port itself. Check the datasheet can be confusing.
*
*/
void MCP23008::setupInterruptPin(uint8_t pin, uint8_t mode) {

	// set the pin interrupt control (0 means change, 1 means compare against given value);
	updateRegisterBit(pin, (mode != CHANGE), MCP23008_INTCON);
	// if the mode is not CHANGE, we need to set up a default value, different value triggers interrupt

	// In a RISING interrupt the default value is 0, interrupt is triggered when the pin goes to 1.
	// In a FALLING interrupt the default value is 1, interrupt is triggered when pin goes to 0.
	updateRegisterBit(pin, (mode == FALLING), MCP23008_DEFVAL);

	// enable the pin for interrupt
	updateRegisterBit(pin, HIGH, MCP23008_GPINTEN);

}

uint8_t MCP23008::getLastInterruptPin() {
	uint8_t intf;

	// try port A
	intf = readRegister(MCP23008_INTF);
	for (int i = 0; i<8; i++) if (bitRead(intf, i)) return i;

	// try port B


	return MCP23008_INT_ERR;

}
uint8_t MCP23008::getLastInterruptPinValue() {
	uint8_t intPin = getLastInterruptPin();
	if (intPin != MCP23008_INT_ERR) {
		uint8_t intcapreg = regForPin(intPin, MCP23008_INTCAP);
		uint8_t bit = bitForPin(intPin);
		return (readRegister(intcapreg) >> bit) & (0x01);
	}

	return MCP23008_INT_ERR;
}


