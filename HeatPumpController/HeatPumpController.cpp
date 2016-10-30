#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "LEDBackpack.h"
#include "Smith_I2C.h"
#include "OW.h"
#include <thread>
#include <mutex>

using namespace std;
pthread_mutex_t wIm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wOm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t aOm = PTHREAD_MUTEX_INITIALIZER;


struct DS18B20_SENSORS {
	uint8_t ROM[8];
	float tempF;
	float last_tempF;
} waterIn = { { 0x28, 0x6C, 0x5B, 0x89, 0x03, 0x00, 0x00, 0xEB } , 0, 0 }, waterOut = { { 0x28, 0x51, 0xAB, 0x6F, 0x03, 0x00, 0x00, 0xAA }, 0, 0 }, room = { { 0x28, 0xA7, 0x75, 0xC2, 0x3, 0x0, 0x0, 0xA2 } , 0, 0 }, airOut = { { 0x28, 0xE2, 0x7D, 0x02, 0x08, 0x00, 0x00, 0xFA } , 0, 0};

void reader();


int main(int argc, char *argv[])
{
	pthread_t readert;
	int rc1;

	int file_i2c;
	int length;
	unsigned char buffer[60] = { 0 };
	float temp, temp2;
	uint8_t address[8];

	//----- OPEN THE I2C BUS -----
	Smith_I2C multiplexer = Smith_I2C();
	if (multiplexer.i2cSetup() < 0)
		return -1;
	multiplexer.directWriteByte(0x70, 1);
	sleep(1);
	std::thread t1(reader);

	//OW onewire = OW();
	//onewire.begin(0x18);
	Smith_7segment redseg = Smith_7segment();
Smith_7segment whiteseg = Smith_7segment();

redseg.begin(0x74, file_i2c);
whiteseg.begin(0x73, file_i2c);
redseg.setBrightness(1);
whiteseg.setBrightness(1);




	cout << endl;

	

	

//	
	Smith_AlphaNum4 textseg = Smith_AlphaNum4();
//	
//	
	textseg.begin(0x72, 0);
	printf("The Temp is %f vs %f \n", temp, temp2);
	
	while (1)
	{
		cout << "while" << endl;
		pthread_mutex_lock(&wIm);
		redseg.print((waterOut.tempF));
		pthread_mutex_unlock(&wIm);
		pthread_mutex_lock(&wOm);
		whiteseg.print((waterIn.tempF));
		pthread_mutex_unlock(&wOm);
		redseg.writeDisplay();
		whiteseg.writeDisplay();
		usleep(250000);
	}


//	redseg.print(6.19);
//	whiteseg.print(1234);
//	textseg.print("COOL");

	textseg.writeDigitAscii(0, 'H', 0);
	textseg.writeDigitAscii(1, '3', 0);
	textseg.writeDigitAscii(2, '2', 0);
	textseg.writeDigitAscii(3, '1', 0);
	textseg.setBrightness(1);
//	redseg.clear();
//	redseg.writeDigitNum(1, 7, false);
//	redseg.writeDisplay();
//	whiteseg.writeDisplay();
	textseg.writeDisplay();
	
	cout << " Test Hello World " << file_i2c << endl;


}



void reader()
{
	DS18B20 OW = DS18B20();
	OW.begin(0x18);


	OW.setResolution(waterIn.ROM, (uint8_t)12);
	OW.setResolution(waterOut.ROM, (uint8_t)12);
	OW.startConversion();


	while(1)
	{
		usleep(750000);
		pthread_mutex_lock(&wIm);
		waterIn.tempF = OW.getTempF(waterIn.ROM);
		pthread_mutex_unlock(&wIm);
		pthread_mutex_lock(&wOm);
		waterOut.tempF = OW.getTempF(waterOut.ROM);
		pthread_mutex_unlock(&wOm);


		OW.startConversion();


	}

	

}