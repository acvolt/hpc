#pragma once


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
#include <time.h>
#include <chrono>
#include "Fram.h"
#include <stdexcept>
#include <csignal>
#include <atomic>

//RELAY STATES (LOW IS ON!!)
#define ON 0
#define OFF 1

//Unit Modes
#define FAN 3
#define COOL 2
#define HEAT 1
#define DISABLED 0

//Operation States
#define OP_STOPPED 0	//Unit stopped waiting for trigger
#define OP_RUNNING 1	//Unit running
#define OP_WAIT 126		//Compressor timeout
#define COOLDOWN 127	//Cooling coils after heat
#define WARMUP 128		//Warming coils prior to turning fan on
#define ALARM 255		//Alarm - causes us to not run again.

#define REVERSER 1		//1 means powered while cooling.
//Setpoints
#define MAX_SETPOINT 120  //Dont' let setpoint exceed 120 F
#define MIN_SETPOINT 35		//Don't let setpoint go below 35 F

#define MAX_HYSTERESIS 10	//Max Hysteresis value
#define MIN_HYSTERESIS 1	//Min Hysteresis value

#define COMP_COOLDOWN 60 //How long between compressor operations

//FRAM Memory Locations
#define TEMP_SETPOINT 1 //FRAM TEMP SETPOINT
#define HYSTERESIS 3 //FRAM HYSTERESIS VALUE
#define LASTMODE 5 //FRAM Last Mode
#define LASTSTATE 7 //FRAM Last State
#define MAXWATERTEMP 9 //FRAM MAX WATER TEMP
#define MINWATERTEMP 11 //FRAM MIN WATER TEMP
#define MAXWATERDELTA 13 //FRAM MAX WATER DELTA
#define COMPRESSORDELAY 15 //LONG (4 Byte)
#define COMPRESSORLASTRUN 20 //(4 Byte) When was the compressor last on
#define COMPRESSORRUNTIME 25 //(4 Byte) How long has the compressor run cumulative
#define WATERVOLUME 30 //(4 Byte) How much water has went past the sensor
#define MAXWATEROUT 35 //What is the max water out reading
#define MINWATEROUT 37 //What is the min water out reading
#define MAXWATERIN 39 //What is the max water in reading
#define MINWATERIN 41 //What is the min water in reading
#define MAXROOM 43 //How hot has the room got?
#define MINROOM 45 //How cool has the room got?
#define MINOUT 47 //How cool has the output got?
#define MAXOUT 49 //How hot has the output got?
#define MINSP 51 //Lowest allowed setpoint
#define MAXSP 53 //Highest allowed setpoint
#define F_ENABLED 55 //DEVICE ENABLED FLAG
#define CHECKVAL 1337 //LEET check to see if we need to re-init the FRAM.


//Alarm Masks
#define ERR_WATER_IN_SENSOR 0x01	//Water Input Temp Sensor issue
#define ERR_WATER_OUT_SENSOR 0x02	//Water Output Temp Sensor issue
#define ERR_ROOM_SENSOR 0x04		//Room Temp Sensor issue
#define ERR_AIR_OUT_SENSOR 0x08	//Air Output Temp Sensor issue
#define ERR_FLOW 0x10				//Water Flow issue
#define ERR_OVERFLOW 0x20			//Condensate Overflow
#define ERR_HIGH_DELTA 0x40		//High Water Delta temperature
#define ERR_LOW_WATEROUT 0x80		//Low Water Output Temperature
#define ERR_HIGH_WATEROUT 0x100	//High Water Output Temperature

#define COMPRESSOR_PIN 13
#define MODE_PIN 19
#define VALVE_PIN 26
#define FAN_PIN 21
#define ENABLED_PIN 20
#define ALARM_PIN 16

std::atomic<bool> sigint = { false };
volatile bool blink = false;
volatile bool overflow_switch; //State of the condensate overflow switch
volatile bool flow_switch; //state of the water flow switch
volatile bool enabled;

volatile uint8_t op_mode = DISABLED; //state of the unit
volatile uint8_t op_state = DISABLED; //Unit able to run?
volatile uint16_t alm = 0;

volatile uint8_t setPoint = 68;
volatile uint8_t hysteresis = 2;

time_t rawtime;
struct tm * now;

volatile time_t compressor_last_start = 0;
volatile time_t compressor_last_stop = 0;

//The next 5 bools hold the current state of the relays.
volatile bool compStat = OFF;
volatile bool modeStat = OFF;
volatile bool valveStat = OFF;
volatile bool fanStat = OFF;
volatile bool almStat = OFF;


char lcdpanel[4][21];



volatile time_t mode_change = 0;  //When did the mode change last?

volatile uint8_t flowcounts;	//how many pulses from the flow meter have we gotten?
volatile uint64_t deciGallons; //tenths of gallons, duh.

DS18B20 OW = DS18B20();







void signalHandler(int signum);
void reader();
void setDisplays();
void buttonISR();
void flowCounter();
void errHandler(uint16_t condition);
void start_unit(uint8_t _op_mode);
void stop_unit(uint8_t _op_mode);
void setRelay(uint8_t relay, bool state);
void monitor();