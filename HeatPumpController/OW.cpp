#include "OW.h"
#include "Smith_I2C.h"
#include <iostream>
#include <stdio.h>
#include <stdexcept>

using namespace std;

Smith_I2C OWi2cDev = Smith_I2C();





OW::OW(){
	mAddress = 0x18;
	mError = 0;
};

OW::~OW(){

};

uint8_t OW::getAddress()
{
	return mAddress;
}

uint8_t OW::getError()
{
	return mError;
}

void OW::s()
{

}

void OW::deviceReset()
{
//	cout << "In 1-Wire Device Reset Function" << endl;
	uint8_t reg[1] = { DS2482_COMMAND_RESET }; //Driver Search command
								 //uint8_t buffer[1] = { 0xF0 }; //OW Search Command
	uint8_t status = 0;
	bool OWB = false; //one wire busy
	OWi2cDev.i2cReadRegByte(0, i2cAddr, reg[0]);
//	cout << "Rst Command has been issued" << endl;
	do
	{
		usleep(100);
		status = OWi2cDev.i2cReadByte(0, i2cAddr);
//		printf("DS2482 Status %02x DIR:%x TSB:%x SBR:%x RST:%x LL:%x SD:%x PPD:%x 1WB:%x \n", status, status >> 7, (status & 0x40) >> 6, (status & 0x20) >> 5, (status & 0x10) >> 4, (status & 0x08) >> 3, (status & 0x04) >> 2, (status & 0x02) >> 1, status & 1);
		OWB = status & DS2482_STATUS_BUSY; //Get the 1WB byte


	} while (OWB);
	status = OWi2cDev.i2cReadByte(0, i2cAddr);
//	printf(" Byte is %2x OWB is %x \n", status, OWB);
//	cout << "exiting 1-wire Device reset" << endl;
	return;
}

void OW::setReadPointer(uint8_t readPointer)
{

//	cout << "In 1-Wire Set Reat Pointer Function" << endl;
	OWi2cDev.i2cWriteByteData(i2cAddr, DS2482_COMMAND_SRP, readPointer);
	//return OWi2cDev.i2cReadRegByte(0, DS2482_COMMAND_SRP);

}

uint8_t OW::readStatus()
{
	setReadPointer(DS2482_POINTER_STATUS);
	return  OWi2cDev.i2cReadByte(0, i2cAddr);
}

uint8_t OW::readData()
{

	setReadPointer(DS2482_POINTER_DATA);
	return  OWi2cDev.i2cReadByte(0, i2cAddr);
}

uint8_t OW::readConfig()
{
	uint8_t readValue[2];
	setReadPointer(DS2482_POINTER_CONFIG);
	readValue[1] = OWi2cDev.i2cReadByte(0, i2cAddr);
	
	return readValue[1];

}

void OW::setStrongPullup()
{
	writeConfig(readConfig() | DS2482_CONFIG_SPU);
}

void OW::clearStrongPullup()
{
	writeConfig(readConfig() & !DS2482_CONFIG_SPU);
}

uint8_t OW::waitOnBusy()
{
	uint8_t status;

	for (int i = 1000; i>0; i--)
	{
		status = readStatus();
		if (!(status & DS2482_STATUS_BUSY))
			break;
		usleep(20);
	}

	// if we have reached this point and we are still busy, there is an error
	if (status & DS2482_STATUS_BUSY) {
		mError = DS2482_ERROR_TIMEOUT;
		printf("DS2482 Error Timeout\n");
	}
//	printf("DS2482 Status %02x DIR:%x TSB:%x SBR:%x RST:%x LL:%x SD:%x PPD:%x 1WB:%x \n", status, status >> 7, (status & 0x40) >> 6, (status & 0x20) >> 5, (status & 0x10) >> 4, (status & 0x08) >> 3, (status & 0x04) >> 2, (status & 0x02) >> 1, status & 1);
	// Return the status so we don't need to explicitly do it again
	return status;
}

void OW::writeConfig(uint8_t config)
{
	waitOnBusy();

//	uint8_t reg = DS2482_COMMAND_WRITECONFIG;
	config = (config | (~config) << 4);
//	OWi2cDev.i2cWriteBlock(i2cAddr, &reg, &buf, 1);
	OWi2cDev.i2cWriteByteData(i2cAddr, DS2482_COMMAND_WRITECONFIG, config);

	// This should return the config bits without the complement
	uint8_t rconfig = OWi2cDev.i2cReadByte(0, i2cAddr);
	if (rconfig != (config & 0x0F)) {
		mError = DS2482_ERROR_CONFIG;
		printf("Didn't read correct config back\n");
		printf("DS2482 Config %02x !WS:%x !SP:%x 1:%x !AP:%x 1WS:%x SP:%x 0:%x AP:%x \n", config, config >> 7, (config & 0x40) >> 6, (config & 0x20) >> 5, (config & 0x10) >> 4, (config & 0x08) >> 3, (config & 0x04) >> 2, (config & 0x02) >> 1, config & 1);
		printf("DS2482 Rconfi %02x !WS:%x !SP:%x 1:%x !AP:%x 1WS:%x SP:%x 0:%x AP:%x \n", rconfig, rconfig >> 7, (rconfig & 0x40) >> 6, (rconfig & 0x20) >> 5, (rconfig & 0x10) >> 4, (rconfig & 0x08) >> 3, (rconfig & 0x04) >> 2, (rconfig & 0x02) >> 1, rconfig & 1);

	}
}

uint8_t OW::wireReset()
{
	waitOnBusy();
	clearStrongPullup();

	waitOnBusy();

//	cout << "In 1-Wire Reset Function" << endl;

	OWi2cDev.i2cReadRegByte(0, i2cAddr, DS2482_COMMAND_RESETWIRE);
//	cout << "Rst Command has been issued" << endl;
	uint8_t status = waitOnBusy();

	if (status & DS2482_STATUS_SD)
	{
		mError = DS2482_ERROR_SHORT;
		printf("Bus Shorted? \n");
	}
//	printf("DS2482 Status %02x DIR:%x TSB:%x SBR:%x RST:%x LL:%x SD:%x PPD:%x 1WB:%x \n", status, status >> 7, (status & 0x40) >> 6, (status & 0x20) >> 5, (status & 0x10) >> 4, (status & 0x08) >> 3, (status & 0x04) >> 2, (status & 0x02) >> 1, status & 1);

	return (status & DS2482_STATUS_PPD) ? true : false;


}

void OW::wireWriteByte(uint8_t data, uint8_t power)
{
	waitOnBusy();
	if (power)
		setStrongPullup();
	uint8_t reg = DS2482_COMMAND_WRITEBYTE;
	OWi2cDev.i2cWriteByteData(i2cAddr, reg, data);

}

uint8_t OW::wireReadByte()
{

	waitOnBusy();

//	OWi2cDev.i2cMultiWriteOneTwo(i2cAddr, registers, 3);
	OWi2cDev.i2cWriteByte(i2cAddr, DS2482_COMMAND_READBYTE);
	waitOnBusy();
	setReadPointer(DS2482_POINTER_DATA);
	return OWi2cDev.i2cReadByte(0, i2cAddr);
}



bool OW::begin(uint8_t addr)
{
	uint8_t byte, config;
	OWi2cDev.i2cSetup();
	i2cAddr = addr;

	deviceReset();
	usleep(500);
	writeConfig(0b11100001);
	config = readConfig();
//	printf("DS2482 Config %02x !WS:%x !SP:%x 1:%x !AP:%x 1WS:%x SP:%x 0:%x AP:%x (begin) \n", config, config >> 7, (config & 0x40) >> 6, (config & 0x20) >> 5, (config & 0x10) >> 4, (config & 0x08) >> 3, (config & 0x04) >> 2, (config & 0x02) >> 1, config & 1);


	


	usleep(100);

	byte = OWi2cDev.i2cReadByte(0, i2cAddr);
//	printf("\nByte is %x buffer is %x \n", byte, buffer[0]);
//	byte = OWi2cDev.i2cReadRegByte(0, i2cAddr, 0xF0);
//	printf("\n Reg Byte is %x \n", byte);

//	OWi2cDev.i2cWriteBlock(i2cAddr, reg, buffer, 1);

	byte = OWi2cDev.i2cReadByte(0, i2cAddr);
//	printf("\nByte (try 2) is %x \n", byte);

//	reg[0] = 0xe1;
//	buffer[1] = 0xc3;
//	OWi2cDev.i2cWriteBlock(i2cAddr, reg, buffer, 1 );
//	byte = OWi2cDev.i2cReadByte(0, i2cAddr);
//	printf("\n Reg Byte (try 2) is %x \n", byte);

	return true;
};

int OW::search() {
	int rslt, i, cnt;
	// find ALL devices
	cout << "\nFIND ALL\n";
//	printf("\nFIND ALL\n");
	cnt = 0;
	rslt = OWFirst();
	while (rslt)
	{
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("  %d\n", ++cnt);

		rslt = OWNext();
	}
	/*
	// find only 0x1A
	printf("\nFIND ONLY 0x1A\n");
	cnt = 0;
	OWTargetSetup(0x1A);
	while (OWNext())
	{
		// check for incorrect type
		if (ROM_NO[0] != 0x1A)
			break;

		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("  %d\n", ++cnt);
	}
	*/


	// find all but 0x04, 0x1A, 0x23, and 0x01
	printf("\nFIND ALL EXCEPT 0x10, 0x04, 0x0A, 0x1A, 0x23, 0x01\n");
	cnt = 0;
	rslt = OWFirst();
	while (rslt)
	{
		// check for incorrect type
		if ((ROM_NO[0] == 0x04) || (ROM_NO[0] == 0x1A) ||
			(ROM_NO[0] == 0x01) || (ROM_NO[0] == 0x23) ||
			(ROM_NO[0] == 0x0A) || (ROM_NO[0] == 0x10))
			OWFamilySkipSetup();
		else
		{
			// print device found
			for (i = 7; i >= 0; i--)
				printf("%02X", ROM_NO[i]);
			printf("  %d\n", ++cnt);
		}

		rslt = OWNext();
	}
	printf("\n Found %d Devices", cnt);
	return cnt;
};


int OW::OWFirst()
{ 
	//reset the search state
	LastDiscrepancy = 0;
	LastDeviceFlag = false;
	LastFamilyDiscrepancy = 0;

	return OWSearch(); 
};

int OW::OWNext()
{ 
	// leave search state alone
	return OWSearch(); 
};

 int OW::OWSearch(/*uint8_t *address*/)
{  /*
	 uint8_t direction;
	 uint8_t last_zero = 0;
	 uint8_t reg[1];
	 uint8_t buffer[1];

	 if (searchLastDeviceFlag) {
		 printf("Exiting OWSearch due to searchLastDeviceFlag \n");
		 return 0;
	 }

	 if (!wireReset())
	 {
		 printf("Exiting OWSearch due to WireReset\n");
		 return 0;
	 }

	 waitOnBusy();

	 wireWriteByte(WIRE_COMMAND_SEARCH);

	 for (uint8_t i = 0; i < 64; i++)
	 {
		 int searchByte = i / 8;
		 int searchBit = 1 << i % 8;

		 if (i < searchLastDiscrepancy)
		 {
			 direction = searchAddress[searchByte] & searchBit;
		 
		 }
		 else
		 {
			 direction = i == searchLastDiscrepancy;
		 }

		 waitOnBusy();


		 reg[0] = DS2482_COMMAND_TRIPLET;
		 buffer[0] = (direction ? 0x80 : 0x00);
		 OWi2cDev.i2cWriteByteData(i2cAddr, reg[0], buffer[0]);




		 uint8_t status = waitOnBusy();

		 uint8_t id = status & DS2482_STATUS_SBR;
		 uint8_t comp_id = status & DS2482_STATUS_TSB;
		 direction = status & DS2482_STATUS_DIR;

		 if (id && comp_id)
		 {
			 return 0;
		 }
		 else
		 {
			 if (!id && !comp_id && !direction)
			 {
				 last_zero = i;
			 }

		 }
		 if (direction)
		 {
			 searchAddress[searchByte] |= searchBit;
		 }
		 else {
			 searchAddress[searchByte] &= ~searchBit;
		 }
		 printf("SA: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x \n", searchAddress[0], searchAddress[1], searchAddress[2], searchAddress[3], searchAddress[4], searchAddress[5], searchAddress[6], searchAddress[7]);
	 }

	 searchLastDiscrepancy = last_zero;

	 if (!last_zero)
		 searchLastDeviceFlag = 1;

	 for (uint8_t i = 0; i < 8; i++)
		 address[i] = searchAddress[i];


	 return 1;
	 */

	 
	int id_bit_number, last_zero, rom_byte_number, search_result, id_bit, cmp_id_bit;
	uint8_t rom_byte_mask, search_direction, reg[1], buffer[1], byte;

//	bool owb;

	//initialize for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;
	crc8 = 0;
	search_direction = 0;

	//if the last call was not the last one
//	printf(" Last Device Flag is %x\n", LastDeviceFlag);
	if (!LastDeviceFlag)
	{
		//1-wire reset
//		cout << "Resetting 1-Wire now (pre-command)" << endl;
		if (!wireReset())
		{
//			printf("Exiting due to wireReset\n");
			//reset the search
			LastDiscrepancy = 0;
			LastDeviceFlag = false;
			LastFamilyDiscrepancy = 0;
			return false;
		}
//		cout << "After Reset" << endl;
		wireWriteByte(WIRE_COMMAND_SEARCH);

		usleep(600); //Wait for command execution

		do {
//			printf(" top Search direction is %d \n", search_direction);

			reg[0] = DS2482_COMMAND_TRIPLET;
			buffer[0] = (search_direction ? 0x80 : 0x00);
			OWi2cDev.i2cWriteByteData(i2cAddr, reg[0], buffer[0]);
			
			byte = waitOnBusy();



//			search_direction = byte >> 7;
			id_bit = (byte & 0b00100000) >> 5;
			cmp_id_bit = (byte & 0b01000000) >> 6;

//			printf(" id is %x cmp_id is %x dir is %x->%x id_bit %d :: %02x %02x %02x %02x %02x %02x %02x %02x Mask %02x\n", id_bit, cmp_id_bit, search_direction, buffer[0], id_bit_number, ROM_NO[0], ROM_NO[1], ROM_NO[2], ROM_NO[3], ROM_NO[4], ROM_NO[5], ROM_NO[6], ROM_NO[7], rom_byte_mask);

			if ((id_bit==1) && (cmp_id_bit)==1)
				break;
			else {
				//all devices could have 0 or 1
				if (id_bit != cmp_id_bit) {
					search_direction = id_bit;
	//				printf("Search direction set to %x\n", search_direction);
				}
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < LastDiscrepancy)
					{
						search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
	//					printf("Search direction set to %x\n", search_direction);

					}
					else
					{
						//if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == LastDiscrepancy);
//						printf("Search direction set to %x\n", search_direction);

					}

					if (search_direction == 0)
					{
						last_zero = id_bit_number;

						if (last_zero < 9)
							LastFamilyDiscrepancy = last_zero;

					}

				}

				//set or clear the bit in the ROM byte rom_byte_numer with mask rom_byte_mask
//				printf("Search direction is %d \n", search_direction);
				if (search_direction == 1)
					ROM_NO[rom_byte_number] |= rom_byte_mask;
				else
					ROM_NO[rom_byte_number] &= ~rom_byte_mask;
//				printf(" id is %x cmp_id is %x id_bit %d :: %02x %02x %02x %02x %02x %02x %02x %02x Mask %02x\n\n", id_bit, cmp_id_bit, id_bit_number, ROM_NO[0], ROM_NO[1], ROM_NO[2], ROM_NO[3], ROM_NO[4], ROM_NO[5], ROM_NO[6], ROM_NO[7], rom_byte_mask);

				//serial number seearch direction write bit (already done by driver)

				//increment the byte counter id_bit_number and shift the rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				//if the mask is 0 t hen go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					docrc8(ROM_NO[rom_byte_number]); //accumulate the CRC
					rom_byte_number++;
					rom_byte_mask = 1;
				}


			}


	
		} while (rom_byte_number < 8);

		//if the search was successful then
		if (!((id_bit_number < 65) || (crc8 != 0)));
		{
			//search successful so set LastDiscrepance, LastDeviceFlag,search_result
			LastDiscrepancy = last_zero;

			//check for last device
			if (LastDiscrepancy == 0)
			{
				LastDeviceFlag = true;

			}
			search_result = true;
//			printf("Maybe we found something? \n ");
		}

	}
	if (!search_result || !ROM_NO[0])
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = false;
		LastFamilyDiscrepancy = 0;
		search_result = false;
	}

	return search_result;
	
}

//Verify the device with ROM number in ROM_NO buffer is present.
//True = Present & verified

int OW::OWVerify()
{
	
	unsigned char rom_backup[8];
	int i, rslt, ld_backup, ldf_backup, lfd_backup;

	// keep a backup copy of the current state
	for (i = 0; i < 8; i++)
		rom_backup[i] = ROM_NO[i];
	ld_backup = LastDiscrepancy;
	ldf_backup = LastDeviceFlag;
	lfd_backup = LastFamilyDiscrepancy;

	//set search to find the same device
	LastDiscrepancy = 64;
	LastDeviceFlag = false;

	if (OWSearch())
	{
		//check if same device found
		rslt = true;
		for (i = 0; i < 8; i++)
		{
			if (rom_backup[i] != ROM_NO[i])
			{
				rslt = false;
				break;
			}
		}
	}
	else
		rslt = false;
	//restore the search state

	for (i = 0; i < 8; i++)
		ROM_NO[i] = rom_backup[i];
	LastDiscrepancy = ld_backup;
	LastDeviceFlag = ldf_backup;
	LastFamilyDiscrepancy = lfd_backup;

	return rslt; 
}; 


//--------------------------------------------------------------------------
// Setup the search to find the device type 'family_code' on the next call
// to OWNext() if it is present.
//
void OW::OWTargetSetup(unsigned char family_code)
{
	int i;

	// set the search state to find SearchFamily type devices
	ROM_NO[0] = family_code;
	for (i = 1; i < 8; i++)
		ROM_NO[i] = 0;
	LastDiscrepancy = 64;
	LastFamilyDiscrepancy = 0;
	LastDeviceFlag = false;
};

//--------------------------------------------------------------------------
// Setup the search to skip the current device type on the next call
// to OWNext().
//
void OW::OWFamilySkipSetup()
{
	// set the Last discrepancy to last family discrepancy
	LastDiscrepancy = LastFamilyDiscrepancy;
	LastFamilyDiscrepancy = 0;

	// check for end of list
	if (LastDiscrepancy == 0)
		LastDeviceFlag = true;
};

//--------------------------------------------------------------------------
// Reset the 1-Wire bus and return the presence of any device
// Return TRUE  : device present
//        FALSE : no device present
//
int OW::OWReset()
{
//	cout << "In 1-Wire Reset Function" << endl;
//	uint8_t reg[1] = { DS2482_COMMAND_RESETWIRE }; //Driver Search command
	//uint8_t buffer[1] = { 0xF0 }; //OW Search Command
	uint8_t status = 0;
	//OWi2cDev.i2cWriteByte(i2cAddr, reg[1]);
	OWi2cDev.i2cReadRegByte(0, i2cAddr, DS2482_COMMAND_RESETWIRE);
//	cout << "Rst Command has been issued" << endl;
	do
	{
		usleep(100);
		status = OWi2cDev.i2cReadByte(0, i2cAddr);
//		printf("DS2482 Status %02x DIR:%x TSB:%x SBR:%x RST:%x LL:%x SD:%x PPD:%x 1WB:%x \n", status, status >> 7, (status & 0x40) >> 6, (status & 0x20) >> 5, (status & 0x10) >> 4, (status & 0x08) >> 3, (status & 0x04) >> 2, (status & 0x02) >> 1, status & 1);

		status = status & 0x01; //Get the 1WB byte


	} while (status);
	status = OWi2cDev.i2cReadByte(0, i2cAddr);
//	printf(" status is %2x \n", status);
//	cout << "exiting 1-wire reset" << endl;
	return 1;
};

void OW::OWWriteByte(unsigned char byte_value)
{
	uint8_t reg[1] = { DS2482_COMMAND_WRITEBYTE }; //Driver Search command
	uint8_t buffer[1] = { 0x33 }; //OW Search Command
	uint8_t byte = 0;
	OWi2cDev.i2cWriteBlock(i2cAddr, reg, buffer, 1);
	do
	{
		usleep(100);
		byte = OWi2cDev.i2cReadByte(0, i2cAddr);
		byte = byte & 0x01; //Get the 1WB byte


	} while (byte);
	buffer[0] = byte_value;
	OWi2cDev.i2cWriteBlock(i2cAddr, reg, buffer, 1);

	return;
}


//--------------------------------------------------------------------------
// Send 1 bit of data to teh 1-Wire bus
//
void OW::OWWriteBit(unsigned char bit_value)
{
	// platform specific

	// TMEX API TEST BUILD

}

//--------------------------------------------------------------------------
// Read 1 bit of data from the 1-Wire bus 
// Return 1 : bit read is 1
//        0 : bit read is 0
//


// TEST BUILD
static unsigned char dscrc_table[] = {
	0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
	157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
	35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
	190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
	70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
	219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
	101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
	248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
	140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
	17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
	175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
	50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
	202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
	87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
	233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
	116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53 };

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current 
// global 'crc8' value. 
// Returns current global crc8 value
//
unsigned char OW::docrc8(unsigned char value)
{
	// See Application Note 27

	// TEST BUILD
	crc8 = dscrc_table[crc8 ^ value];
	return crc8;
}


void DS18B20::setResolution(uint8_t* ROM, uint8_t resolution)
{
	uint8_t config_register;
	uint8_t scratchpad[9];
	uint8_t i;
	switch (resolution) {
	case 9://93.75ms conversion time .5C
		config_register = 0b00011111;
		break;
	case 10://187.5ms conversion time .25C
		config_register = 0b00111111;
		break;
	case 11: //375ms conversion time .125C
		config_register = 0b01011111;
		break;
	case 12: //750ms conversion time .0625C resolution
		config_register = 0b01111111;
		break;
	}
	readScratchpad(ROM, scratchpad);

	scratchpad[4] = config_register;

	selectROM(ROM);

	wireWriteByte(DS18B20_WRITE_SCRATCHPAD);
	for (i = 2; i < 5; i++)
	{
		wireWriteByte(scratchpad[i]);
	}

	wireReset();
}

void DS18B20::readScratchpad(uint8_t* ROM, uint8_t* scratchpad)
{
	
	selectROM(ROM);
	wireWriteByte(DS18B20_READ_SCRATCHPAD);
//	printf("In readScratchpad and getting");
	for (int i = 0; i < 9; i++)
	{
		scratchpad[i] = wireReadByte();
//		printf(":%02x", scratchpad[i]);
	}
//	printf("\n");
}

void DS18B20::readScratchpad(uint8_t* ROM, uint8_t* scratchpad, uint8_t length)
{
	selectROM(ROM);
	wireWriteByte(DS18B20_READ_SCRATCHPAD);
	printf("In readScratchpad length %d and getting", length);
	for (int i = 0; i < length; i++)
	{
		scratchpad[i] = wireReadByte();
		printf(":%02x", scratchpad[i]);
	}
	printf("\n");
}

void DS18B20::selectROM(uint8_t* ROM)
{
	wireReset();
	wireWriteByte(OW_MATCH_ROM);
	usleep(620);
	for (int i = 0; i < 8; i++)
	{
		wireWriteByte(ROM[i]);
		waitOnBusy();
	}
}

void DS18B20::skipROM()
{
	wireReset();
	wireWriteByte(OW_SKIP_ROM);
	usleep(620);
}

void DS18B20::readROM()
{
	wireReset();
	wireWriteByte(OW_READ_ROM);
	usleep(620);
}

void DS18B20::startConversion()
{
	skipROM();
	wireWriteByte(DS18B20_CONVERT);
}

void DS18B20::startConversion(uint8_t* ROM)
{
	selectROM(ROM);
	wireWriteByte(DS18B20_CONVERT);
}

float DS18B20::getTempF(uint8_t* ROM)
{
	uint8_t scratchpad[9];
	readScratchpad(ROM, scratchpad);

	short int temperature;
	temperature = ((scratchpad[1] << 8 | scratchpad[0]));
//	printf("\n gtf temperature is %x at %x \n ", temperature, ROM[0]);
	if (temperature == 0xffffffff) {
		printf("\n Read Error device %x \n", ROM[7]);
		throw std::__throw_out_of_range;
		return -40;
	}
	float conversion;
	if (scratchpad[1] & 0x80)
	{
		conversion = (float)-(~(temperature - 1) / 16);
	}
	else
		conversion = (float)(temperature / 16);

	

	if (temperature <= DEVICE_DISCONNECTED_RAW)
	{
		return DEVICE_DISCONNECTED_F;
	}
//	printf("ROM[1] %x Temperature is %f\n", ROM[1], ((conversion*1.8 + 32)));
//	return((float)(temperature.temperature * 0.0140625) + 32);
	return ((conversion*1.8)+32);
	//First byte is Temp LSB, 2nd is Temp MSB



}

float DS18B20::getTempC(uint8_t* ROM)
{
	return -999.1;
}


