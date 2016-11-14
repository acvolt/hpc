#pragma once
#include <stdlib.h>
#include <math.h>
#include <wiringPiSPI.h>
#include <inttypes.h>



#ifndef _FRAM_SPI_H
#define _FRAM_SPI_H


typedef enum opcodes_e
{
	OPCODE_WREN = 0b0110,     /* Write Enable Latch */
	OPCODE_WRDI = 0b0100,     /* Reset Write Enable Latch */
	OPCODE_RDSR = 0b0101,     /* Read Status Register */
	OPCODE_WRSR = 0b0001,     /* Write Status Register */
	OPCODE_READ = 0b0011,     /* Read Memory */
	OPCODE_WRITE = 0b0010,     /* Write Memory */
	OPCODE_RDID = 0b10011111  /* Read Device ID */
} opcodes_t;


class Smith_FRAM_SPI {
public:
	Smith_FRAM_SPI(bool bus, bool cs);


	bool  begin(void);
	void     writeEnable(bool enable);
	void     write8(uint16_t addr, uint8_t value);
	void     write(uint16_t addr, const uint8_t *values, size_t count);
	uint32_t readlong(uint16_t addr);
	uint8_t  read8(uint16_t addr);
	void     getDeviceID(uint8_t *manufacturerID, uint16_t *productID);
	uint8_t  getStatusRegister(void);
	void     setStatusRegister(uint8_t value);

private:
	uint8_t  SPItransfer(uint8_t x);

	bool _framInitialised;
	bool _cs, _bus;
	int _spi;
};





#endif