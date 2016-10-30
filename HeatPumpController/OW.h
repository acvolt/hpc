#pragma once
#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#include <stdint.h>

//Defines from cybergibbons
#define DS2482_COMMAND_RESET			0xF0	//reset ds2482 chip itself
#define DS2482_COMMAND_SRP				0xE1	//set read pointer
#define DS2482_POINTER_STATUS			0xF0	//Status Pointer
#define DS2482_STATUS_BUSY				(1<<0)	//1WB status
#define DS2482_STATUS_PPD				(1<<1)	//presence pulse detect
#define DS2482_STATUS_SD				(1<<2)	//short detect
#define DS2482_STATUS_LL				(1<<3)	//1WB logic level (no activity)
#define DS2482_STATUS_RST				(1<<4)	//Has the device reset
#define DS2482_STATUS_SBR				(1<<5)	//Single Bit Read
#define DS2482_STATUS_TSB				(1<<6)	//Triplet Second Bit
#define DS2482_STATUS_DIR				(1<<7)	//Search Direction

#define DS2482_POINTER_DATA				0xE1
#define DS2482_POINTER_CONFIG			0xC3
#define DS2482_CONFIG_APU				(1<<0)
#define DS2482_CONFIG_SPU				(1<<2)
#define DS2482_CONFIG_1WS				(1<<3)

#define DS2482_COMMAND_WRITECONFIG		0xD2
#define DS2482_COMMAND_RESETWIRE		0xB4
#define DS2482_COMMAND_WRITEBYTE		0xA5
#define DS2482_COMMAND_READBYTE			0x96
#define DS2482_COMMAND_SINGLEBIT		0x87
#define DS2482_COMMAND_TRIPLET			0x78
	
#define WIRE_COMMAND_SKIP				0xCC
#define WIRE_COMMAND_SELECT				0x55
#define WIRE_COMMAND_SEARCH				0xF0
	
#define DS2482_ERROR_TIMEOUT			(1<<0)
#define DS2482_ERROR_SHORT				(1<<1)
#define DS2482_ERROR_CONFIG				(1<<2)

#define OW_READ_ROM						0x33
#define OW_MATCH_ROM					0x55
#define OW_SKIP_ROM						0xCC

#define DS18B20_CONVERT					0x44
#define DS18B20_WRITE_SCRATCHPAD		0x4E
#define DS18B20_READ_SCRATCHPAD			0xBE
#define DS18B20_COPY_SCRATCHPAD			0x48
#define DS18B20_RECALL_EEPROM			0xB8
#define DS18B20_READ_POWER				0xB4
#define DS18B20_ALARM_SEARCH			0xEC

#define DEVICE_DISCONNECTED_C -127
#define DEVICE_DISCONNECTED_F -196.6
#define DEVICE_DISCONNECTED_RAW -7040


class OW
{
public:
	OW();
	~OW();
	int search();
	int OWSearch(uint8_t *address);
	int OWSearch();
	bool begin(uint8_t addr);
	uint8_t getAddress();
	uint8_t getError();
	uint8_t checkPresence();

	void deviceReset();
	void setReadPointer(uint8_t readPointer);
	uint8_t readStatus();
	uint8_t readData();
	uint8_t waitOnBusy();
	uint8_t readConfig();

	void writeConfig(uint8_t config);
	void setStrongPullup();
	void clearStrongPullup();
	uint8_t wireReset();
	void wireWriteByte(uint8_t data, uint8_t power = 0);
	uint8_t wireReadByte();
	void wireWriteBit(uint8_t data, uint8_t power = 0);
	uint8_t wireReadBit();
	void wireSkip();
	void wireSelect(const uint8_t rom[8]);
	void wireResetSearch();
	uint8_t wireSearch(uint8_t *address);

	// emulation of original OneWire library
	void reset_search();
	uint8_t search(uint8_t *newAddr);
	static uint8_t scrc8(const uint8_t *addr, uint8_t len);
	uint8_t reset(void);
	void select(const uint8_t rom[8]);
	void skip(void);
	void write(uint8_t v, uint8_t power = 0);
	uint8_t read(void);
	uint8_t read_bit(void);
	void write_bit(uint8_t v);

protected:
	int OWFirst();
	int OWNext();

	int OWVerify();
	void OWTargetSetup(uint8_t family_code);
	void OWFamilySkipSetup();
	int OWReset();
	void OWWriteByte(uint8_t byte_value);
	void OWWriteBit(uint8_t vit_value);
	uint8_t OWReadbit();
	uint8_t docrc8(uint8_t value);

	uint8_t ROM_NO[8];
	int LastDiscrepancy;
	int LastFamilyDiscrepancy;
	int LastDeviceFlag;
	uint8_t crc8;
	uint8_t i2cAddr;


private:
	void s();
	uint8_t end();
	void writeByte(uint8_t);
	uint8_t readByte();

	uint8_t mAddress;
	uint8_t mError;

	uint8_t searchAddress[8];
	uint8_t searchLastDiscrepancy;
	uint8_t searchLastDeviceFlag;
};


class DS18B20 : public OW
{
public:
	void setResolution(uint8_t* ROM, uint8_t resolution);


	void startConversion();
	void startConversion(uint8_t* ROM);
	void setAlarm(uint8_t High, uint8_t Low);
	void setAlarm(uint8_t* ROM, uint8_t High, uint8_t Low);
	float getTempF(uint8_t* ROM);
	float getTempC(uint8_t* ROM);
	int getTempF10x(uint8_t* ROM);
	int getTempC10x(uint8_t* ROM);


private:
	void selectROM(uint8_t* ROM);
	void skipROM();
	void readROM();
	void readScratchpad(uint8_t* ROM, uint8_t* scratchpad, uint8_t length);
	void readScratchpad(uint8_t* ROM, uint8_t* scratchpad);

};

#endif
