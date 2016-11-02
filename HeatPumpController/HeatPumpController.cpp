#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "LEDBackpack.h"
#include "Smith_I2C.h"
#include "OW.h"
#include <thread>
#include <mutex>
#include <string>
#include "MCP23008.h"
#include <wiringPi.h>
#include "LiquidCrystal.h"
//#include <wiringPiI2C.h>
//#include <mcp23008.h>

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
void setDisplays();


int main(int argc, char *argv[])
{




	



	//----- OPEN THE I2C BUS -----
//	Smith_I2C multiplexer = Smith_I2C();
//	if (multiplexer.i2cSetup() < 0)
//		return -1;
//	multiplexer.directWriteByte(0x70, 1);
//	multiplexer.~Smith_I2C();

//	sleep(1);
//	std::thread t1(reader);
//	std::thread t2(setDisplays);

	//bool blink = false;

	cout << "Before" << endl;
	Adafruit_LiquidCrystal lcd(0x24);
	cout << "Just init'd" << endl;
	lcd.begin(20, 4);
	cout << "Just begin" << endl;
	lcd.setBacklight(0);
	sleep(1);
	lcd.setBacklight(0);
	cout << "just set backlight just set backlight just set backlight just set backlight" << endl;

	lcd.setCursor(0, 0);

	cout << "just set cursor" << endl;
	lcd.print("Test 2");
	cout << "just lcd.print" << endl;

	/*
	MCP23008 buttonChip;
	buttonChip.begin(0x21);
	bool blink = false;

	buttonChip.pullUp(0, 1);
	buttonChip.pullUp(1, 1);
	buttonChip.pullUp(2, 1);
	buttonChip.pullUp(3, 1);
	buttonChip.pullUp(4, 1);
	buttonChip.pullUp(5, 1);
	buttonChip.pinMode(0, INPUT);
	buttonChip.pinMode(1, INPUT);
	buttonChip.pinMode(2, INPUT);
	buttonChip.pinMode(3, INPUT);
	buttonChip.pinMode(4, INPUT);
	buttonChip.pinMode(5, INPUT);






	buttonChip.pinMode(6, OUTPUT);
	buttonChip.pinMode(7, OUTPUT);

	buttonChip.setupInterrupts(false, false, false);
	buttonChip.setupInterruptPin(0, 2);
	buttonChip.setupInterruptPin(1, 2);


	*/
	
	wiringPiSetupGpio();
	pinMode(6, INPUT);

	/*
	mcp23008Setup(100, 0x21);

	pinMode(100, INPUT);
	pinMode(101, INPUT);
	pinMode(102, INPUT);
	pinMode(103, INPUT);
	pinMode(104, INPUT);
	pinMode(105, INPUT);
	pinMode(106, OUTPUT);
	pinMode(107, OUTPUT);

	*/



	//OW onewire = OW();
	//onewire.begin(0x18);

	while (1)
	{
//		printf("GPIO is %x Last Interrupt is %d Int pin is %x \n", buttonChip.readGPIO(), buttonChip.getLastInterruptPin(), digitalRead(6));
//		buttonChip.digitalWrite(7, blink);
//		buttonChip.digitalWrite(6, !blink);
//		buttonChip.readGPIO();
//
//		digitalWrite(106, blink);
//		digitalWrite(107, !blink);
//		blink = !blink;
		usleep(500000);

	}



	cout << endl;

	

	

//	

//	
//	


	



//	redseg.print(6.19);
//	whiteseg.print(1234);
//	textseg.print("COOL");


//	redseg.clear();
//	redseg.writeDigitNum(1, 7, false);
//	redseg.writeDisplay();
//	whiteseg.writeDisplay();

	
	cout << " Test Hello World " << endl;


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
		usleep(800000);

/*
		pthread_mutex_lock(&wIm);
		waterIn.tempF = OW.getTempF(waterIn.ROM);
		pthread_mutex_unlock(&wIm);

		pthread_mutex_lock(&wOm);
		waterOut.tempF = OW.getTempF(waterOut.ROM);
		pthread_mutex_unlock(&wOm);
		printf("Water Out Temp is %f\n", waterOut.tempF);
		printf("Water In Temp is %f\n", waterIn.tempF);


		pthread_mutex_lock(&rm);
		room.tempF = OW.getTempF(room.ROM);
		pthread_mutex_unlock(&rm);

//		pthread_mutex_lock(&rm);
//		waterOut.tempF = OW.getTempF(room.ROM);
//		pthread_mutex_unlock(&rm);
		OW.startConversion();
		*/

	}

	

}

void setDisplays()
{
	/*
	Smith_7segment redseg = Smith_7segment();
//	Smith_7segment whiteseg = Smith_7segment();
	Smith_7segment blueseg = Smith_7segment();
	Smith_7segment greenseg = Smith_7segment();
//	Smith_AlphaNum4 modeseg = Smith_AlphaNum4();

//	modeseg.begin(0x72, 0);
//	whiteseg.begin(0x73, 0);
	redseg.begin(0x74, 0);
	blueseg.begin(0x75, 0);
	greenseg.begin(0x76, 0);

	
	redseg.setBrightness(5);
//	whiteseg.setBrightness(5);
	blueseg.setBrightness(5);
	greenseg.setBrightness(5);
//	modeseg.setBrightness(1);

	*/

	while (1)
	{
		/*
//		cout << "while" << endl;
		pthread_mutex_lock(&wIm);
		redseg.print((waterOut.tempF));
		pthread_mutex_unlock(&wIm);
	
		pthread_mutex_lock(&wOm);
		blueseg.print((waterIn.tempF));
		pthread_mutex_unlock(&wOm);

		pthread_mutex_lock(&rm);
		greenseg.print((room.tempF));
		pthread_mutex_unlock(&rm);

//		blueseg.print(1111);
//		yellowseg.print(1111);
		redseg.writeDisplay();
//		whiteseg.writeDisplay();
		blueseg.writeDisplay();
		greenseg.writeDisplay();

		*/
		usleep(250000);


//		modeseg.writeDigitAscii(0, 'H', 0);
//		modeseg.writeDigitAscii(1, '3', 0);
//		modeseg.writeDigitAscii(2, '2', 0);
//		modeseg.writeDigitAscii(3, '1', 0);
		
//		modeseg.writeDisplay();
	}

}