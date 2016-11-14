#include "Fram.h"



Smith_FRAM_SPI::Smith_FRAM_SPI(bool cs, bool bus)
{
	_cs = cs;
	_bus = bus;
	_framInitialised = false;
}

bool Smith_FRAM_SPI::begin(void)
{
	_spi =  wiringPiSPISetup(_bus, 20000000);

	uint8_t manufID;
	uint16_t prodID;

	getDeviceID(&manufID, &prodID);

	if (manufID != 0x04)
	{
		//Unexpected manufacturer
		return false;
	}
	if (prodID != 0x0302)
	{
		//unexpected prod id
		return false;
	}
	_framInitialised = true;
	return true;
}

void Smith_FRAM_SPI::writeEnable(bool enable)
{
	uint8_t opcode;
	if (enable)
	{
		opcode = OPCODE_WREN;
	}
	else {
		opcode = OPCODE_WRDI;
	}
	wiringPiSPIDataRW(_cs, &opcode, 1);
}

void Smith_FRAM_SPI::write8(uint16_t addr, uint8_t value)
{
	uint8_t buffer[4];
	buffer[0] = OPCODE_WRITE;
	buffer[1] = ((uint8_t)(addr >> 8));
	buffer[2] = ((uint8_t)(addr & 0xFF));
	buffer[3] = value;

	wiringPiSPIDataRW(_cs, buffer, 4);

}

void Smith_FRAM_SPI::write(uint16_t addr, const uint8_t * values, const size_t count)
{
	uint8_t buffer[8192];
	buffer[0] = OPCODE_WRITE;
	buffer[1] = ((uint8_t)(addr >> 8));
	buffer[2] = ((uint8_t)(addr & 0xFF));
	for (int i = 3; i < (count + 3); i++)
	{
		buffer[i] = values[i-3];
	}
	wiringPiSPIDataRW(_cs, buffer, count + 3);
}

uint8_t Smith_FRAM_SPI::read8(uint16_t addr)
{
	uint8_t buffer[4];
	buffer[0] = OPCODE_READ;
	buffer[1] = ((uint8_t)(addr >> 8));
	buffer[2] = ((uint8_t)(addr & 0xFF));
	buffer[3] = 0;

	wiringPiSPIDataRW(_cs, buffer, 4);

	return buffer[3];
}

uint32_t Smith_FRAM_SPI::readlong(uint16_t addr)
{
	uint8_t buffer[5];
	buffer[0] = OPCODE_READ;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	buffer[4] = 0;

	wiringPiSPIDataRW(_cs, buffer, 5);


	return (uint32_t)((buffer[4] << 24) + (buffer[3] << 16) + (buffer[2] << 8) + (buffer[1] & 0xFF));
}

void Smith_FRAM_SPI::getDeviceID(uint8_t *manufacturerID, uint16_t *productID)
{
	uint8_t a[5] = { OPCODE_RDID, 0, 0, 0, 0 };
	uint8_t results;

	wiringPiSPIDataRW(_cs, a, 5);


	/* Shift values to separate manuf and prod IDs */
	/* See p.10 of http://www.fujitsu.com/downloads/MICRO/fsa/pdf/products/memory/fram/MB85RS64V-DS501-00015-4v0-E.pdf */
	*manufacturerID = (a[1]);
	*productID = (a[3] << 8) + a[4];
}

uint8_t Smith_FRAM_SPI::getStatusRegister(void)
{
	uint8_t buffer[2];
	buffer[0] = OPCODE_RDSR;
	buffer[1] = 0;
	wiringPiSPIDataRW(_cs, buffer, 2);


	return buffer[1];
}

void Smith_FRAM_SPI::setStatusRegister(uint8_t value)
{
	uint8_t buffer[2];
	buffer[0] = OPCODE_WRSR;
	buffer[1] = value;
	wiringPiSPIDataRW(_cs, buffer, 2);


	return;
}

