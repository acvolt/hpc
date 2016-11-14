
#include "HeatPumpController.h"

//#include <wiringPiI2C.h>
//#include <mcp23008.h>

using namespace std;
using namespace std::chrono;
pthread_mutex_t wIm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wOm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t aOm = PTHREAD_MUTEX_INITIALIZER;


struct DS18B20_SENSORS {
	uint8_t ROM[8];
	float tempF;
	float last_tempF;
	unsigned int read_index;
	unsigned int last_error_index;
	unsigned int err_cnt;
} waterIn = { { 0x28, 0x6C, 0x5B, 0x89, 0x03, 0x00, 0x00, 0xEB } , 0, 0, 0, 0, 0 }, waterOut = { { 0x28, 0x51, 0xAB, 0x6F, 0x03, 0x00, 0x00, 0xAA }, 0, 0, 0, 0, 0 }, room = { { 0x28, 0xA7, 0x75, 0xC2, 0x3, 0x0, 0x0, 0xA2 } , 0, 0, 0, 0, 0 }, airOut = { { 0x28, 0xE2, 0x7D, 0x02, 0x08, 0x00, 0x00, 0xFA } , 0, 0, 0, 0, 0 };




Smith_I2C multiplexer = Smith_I2C(0x70);
Adafruit_LiquidCrystal lcd(0x24);
MCP23008 buttonChip;
Smith_FRAM_SPI fram = Smith_FRAM_SPI(0, 0);

void signalHandler(int signum)
{
	cout << "Interrupt signal (" << signum << ") received.\n";
	digitalWrite(COMPRESSOR_PIN, 1);
	digitalWrite(MODE_PIN, 1);
	digitalWrite(VALVE_PIN, 1);
	digitalWrite(FAN_PIN, 1);
	digitalWrite(ENABLED_PIN, 1);
	digitalWrite(ALARM_PIN, 1);
	sigint = true;
	
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler);
	wiringPiSetupGpio();
	wiringPiISR(5, INT_EDGE_FALLING, flowCounter);
	wiringPiISR(6, INT_EDGE_FALLING, buttonISR);
	pinMode(5, INPUT);
	pinMode(6, INPUT);

	digitalWrite(COMPRESSOR_PIN, 1);
	digitalWrite(MODE_PIN, 1);
	digitalWrite(VALVE_PIN, 1);
	digitalWrite(FAN_PIN, 1);
	digitalWrite(ENABLED_PIN, 1);
	digitalWrite(ALARM_PIN, 1);

	pinMode(COMPRESSOR_PIN, OUTPUT);
	pinMode(MODE_PIN, OUTPUT);
	pinMode(VALVE_PIN, OUTPUT);
	pinMode(FAN_PIN, OUTPUT);
	pinMode(ENABLED_PIN, OUTPUT);
	pinMode(ALARM_PIN, OUTPUT);





	
	if (fram.begin()) {
		cout << "Found FRAM " << endl;
	}
	else {
		cout << "No FRAM Found" << endl;
		return -1;
	}


	uint8_t value;
	//Read in the last Hysteresis value, if in limits keep it.
	value = fram.read8(HYSTERESIS);
	if ((value < MIN_HYSTERESIS) || (value > MAX_HYSTERESIS))
	{
		hysteresis = 2;
	}
	else
		hysteresis = value;
	value = fram.read8(MINSP);
	if ((value < MIN_SETPOINT) || (value > MAX_SETPOINT));
	{
		fram.writeEnable(true);
		fram.write8(MINSP, 40);
		fram.writeEnable(false);
	}
	value = fram.read8(MAXSP);
	if ((value < MIN_SETPOINT) || (value > MAX_SETPOINT));
	{
		fram.writeEnable(true);
		fram.write8(MINSP, 120);
		fram.writeEnable(false);
	}

	compressor_last_stop = (time_t)fram.readlong(COMPRESSORLASTRUN);
	
	//Read in the last setPoint.  If the value is within limits, keep it.
		value = fram.read8(TEMP_SETPOINT);
	if ((value < fram.read8(MINSP)) || (value > fram.read8(MAXSP)))
		setPoint = 72;
	else
		setPoint = value;
	//Read in the last mode.  If the value makes sense, keep it.
	value = fram.read8(LASTMODE);
	if (value < 4)
		op_mode = value;
	else
		op_mode = DISABLED;
	//Read in the last state.  If not 0, 1, or 255 set to 0 otherwise copy intact
	value = fram.read8(LASTSTATE);
	switch (value)
	{
		case 0:				//Unit not running
		case 1:				//Unit running
		case 126:			//Compressor waiting
		case 127:			//Cooldown
		case 255:			//Alarm
			op_state = value;
			break;
		default:
			op_state = 0;
	}

	if (true)
	{
		fram.writeEnable(true);
		fram.write8(MINWATERIN, 0xFF);
		fram.writeEnable(false);
		printf("FRAM Temp Setpoint is %d\n", fram.read8(TEMP_SETPOINT));
		printf("FRAM Hysteresis is %d\n", fram.read8(HYSTERESIS));
		printf("Last Mode was %d and Last state was %d\n", fram.read8(LASTMODE), fram.read8(LASTSTATE));
		printf("Max Water Temp is %d and Min Water Temp is %d", fram.read8(MAXWATERTEMP), fram.read8(MINWATERTEMP));
		printf("Max Water Out reading %d MIN Water Out Reading %d\n", fram.read8(MAXWATEROUT), fram.read8(MINWATEROUT));
		printf("Max Water IN reading %d MIN Water IN reading %d\n", fram.read8(MAXWATERIN), fram.read8(MINWATERIN));


	}
	value - fram.read8(F_ENABLED);
	if (value)
		enabled = true;
	else
		enabled = false;
	if (enabled)
		setRelay(ENABLED_PIN, !enabled);

	



	//----- OPEN THE I2C BUS -----
	if (multiplexer.i2cSetup() < 0)
		return -1;
	multiplexer.directWriteByte(0x70, 1);



	OW.begin(0x18);
	lcd.begin(20, 4);
	sprintf(lcdpanel[0], "   ONE WIRE SETUP   ");
	lcd.setCursor(0, 0);
	lcd.print(lcdpanel[0]);

	OW.setResolution(waterIn.ROM, 12);
	OW.setResolution(waterOut.ROM, 12);
	OW.setResolution(room.ROM, 12);
	OW.setResolution(airOut.ROM, 12);
	OW.startConversion();
	sleep(1);
	bool tvalidity = false;
	for (int i = 0; i < 10; i++)
	{
		waterIn.tempF = OW.getTempF(waterIn.ROM);
		if ((waterIn.tempF > 120) || (waterIn.tempF < 1))
		{
			tvalidity = false;
		}
		else
			tvalidity = true;
		waterOut.tempF = OW.getTempF(waterOut.ROM);
		if ((waterOut.tempF > 120) || (waterOut.tempF < 1))
		{
			tvalidity = false;
		}
		else
			tvalidity &= true;
		room.tempF = OW.getTempF(room.ROM);
		if ((room.tempF > 120) || (room.tempF < 1))
		{
			tvalidity = false;
		}
		else
			tvalidity &= true;
		airOut.tempF = OW.getTempF(airOut.ROM);
		if ((airOut.tempF > 120) || (airOut.tempF < 1))
		{
			tvalidity = false;
		}
		else
			tvalidity &= true;
		snprintf(lcdpanel[1], 21, "Water I/O:%3.1f/%3.1f", waterIn.tempF, waterOut.tempF);
		snprintf(lcdpanel[2], 21, "Air I/O:%3.1f/%3.1f", room.tempF, airOut.tempF);
		cout << endl;
		cout << "LCD line 1 " << lcdpanel[1] << " end line " << endl;
		cout << "LCD line 2 " <<lcdpanel[2] << " end line " << endl;
		lcd.setCursor(0, 1);
		lcd.print(lcdpanel[1]);
		lcd.setCursor(0, 2);
		lcd.print(lcdpanel[2]);
		lcd.setCursor(0, 3);
		lcd.print(i);
		OW.startConversion();
		sleep(1);
		if (tvalidity)
			break;
	}
	if (!tvalidity)
	{
		//At least 1 sensor is reading invalid!
		return -1;
	}

	lcd.clear();


	

	buttonChip.begin(0x21);


	buttonChip.pullUp(0, 0);
	buttonChip.pullUp(1, 0);
	buttonChip.pullUp(2, 0);
	buttonChip.pullUp(3, 0);
	buttonChip.pullUp(4, 0);
	buttonChip.pullUp(5, 0);
	buttonChip.pinMode(0, INPUT);
	buttonChip.pinMode(1, INPUT);
	buttonChip.pinMode(2, INPUT);
	buttonChip.pinMode(3, INPUT);
	buttonChip.pinMode(4, INPUT);
	buttonChip.pinMode(5, INPUT);






	buttonChip.pinMode(6, OUTPUT);
	buttonChip.pinMode(7, OUTPUT);

	buttonChip.setupInterrupts(false, false, false);
	buttonChip.setupInterruptPin(0, 0);
	buttonChip.setupInterruptPin(1, 0);
	buttonChip.setupInterruptPin(2, 0);
	buttonChip.setupInterruptPin(3, 0);



	
	std::thread t1(reader);
	std::thread t2(setDisplays);

	time_t loop_time;
	time_t loop_time_millis;
	float tempTemp;

	while (1)
	{
		loop_time = std::time(0);
		loop_time_millis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		pthread_mutex_lock(&rm);
		tempTemp = room.tempF;
		pthread_mutex_unlock(&rm);
		if ((loop_time - mode_change) < 10)
		{
			//Wait for any additional mode changes to occur.  Don't want to turn thing on unless mode is static.
			printf("Sleeping for %d seconds after mode change \n", (10000 - (loop_time_millis - mode_change))/1000);
			sleep((10000 - (loop_time_millis - mode_change))/1000);
		}

		switch (op_mode)
		{
		case DISABLED:
			if ((op_state != OP_STOPPED)) { //Unit may be running when off is requested to stop the unit.
				stop_unit(op_mode);
			}
			break;
		case HEAT:
		{
			//We really don't care if we are in heat mode and the temp is within the dead band.
			if ((tempTemp > (setPoint + hysteresis)) || (tempTemp < (setPoint - hysteresis)))
				switch (op_state)
				{
				case OP_STOPPED:
				//Unit is not running.  Need to determine if we need to start the unit.
					if (tempTemp < (setPoint - hysteresis))
					{
						//Need to start unit
						start_unit(op_mode);
					}
					break;
				case OP_RUNNING:
					//Unit is running.  Do we need to stop the unit?
					if (tempTemp > (setPoint + hysteresis))
					{
						stop_unit(op_mode);
					}
					break;
				case OP_WAIT:
					//This is tricky.  We requested a start, but compressor wasn't ready.
					printf("Waiting on Compressor \n");
					sleep(1);
					break;
				case COOLDOWN:
					//We are running the fan after the blower has turned off.  
					break;
				case WARMUP:
					break;
			}
		}
			break;
		case COOL:
			if ((tempTemp > (setPoint + hysteresis)) || (tempTemp < setPoint - hysteresis))
				switch (op_state)
				{
				case OP_STOPPED:
					break;
				case OP_RUNNING:
					break;
				case OP_WAIT:
					break;
				case COOLDOWN:
					break;
				case WARMUP:  //We should never ever ever match this!
					break;
				}
			break;
		case FAN:
			switch (op_state)
			{
			case OP_STOPPED:
				printf("Starting FAN \n");
				start_unit(op_mode);
				break;
			}
			break;
		default:
			printf("In Default clause wtf OP_Mode is %d \n", op_mode);
			stop_unit(op_mode);

		}




		if (sigint)
			break;
		usleep(250000);

	}


	t1.join();
	t2.join();
	if (sigint)
		return -1;

}



void reader()
{


	float temp[4];
	

	OW.startConversion();


	while(1)
	{
		if (sigint)
		{
			return;
		}
		usleep(800000);
		waterIn.read_index++;
		waterOut.read_index++;
		room.read_index++;
		airOut.read_index++;

		//Read the 4 sensors.  If we catch an exception use the last good value.
		try {
			temp[0] = OW.getTempF(waterIn.ROM);
		}
		catch (const std::out_of_range& e)
		{
			cout << "Water In is Out of range:" << e.what() << endl;
			temp[0] = waterIn.tempF;
			if (waterIn.last_error_index == (waterIn.read_index - 1))
			{
				//We have a consecutive error
				waterIn.err_cnt++;
			}
			else
				waterIn.err_cnt = 1;
			if (waterIn.err_cnt > 60) {
				waterIn.tempF = 255;	//signal a bad reading.  We covered it for the first 60 mark it as bad now.
				errHandler(ERR_WATER_IN_SENSOR);
			}
		}

		try {
			temp[1] = OW.getTempF(waterOut.ROM);
		}
		catch (const std::out_of_range& e)
		{
			cout << "Water Out is Out of range:" << e.what() << endl;
			temp[1] = waterIn.tempF;
			if (waterOut.last_error_index == (waterOut.read_index - 1))
			{
				//We have a consecutive error
				waterOut.err_cnt++;
			}
			else
				waterOut.err_cnt = 1;
			if (waterOut.err_cnt > 60)
			{
				waterOut.tempF = 255;	//signal a bad reading.  We covered it for the first 60 mark it as bad now.
				errHandler(ERR_WATER_OUT_SENSOR);
			}
		}

		try {
			temp[2] = OW.getTempF(room.ROM);
		}
		catch (const std::out_of_range& e)
		{
			cout << "Room is Out of range:" << e.what() << endl;
			temp[2] = room.tempF;
			if (room.last_error_index == (room.read_index - 1))
			{
				//We have a consecutive error
				room.err_cnt++;
			}
			else
				room.err_cnt = 1;
			if (room.err_cnt > 60)
			{
				room.tempF = 255;	//signal a bad reading.  We covered it for the first 60 mark it as bad now.
				errHandler(ERR_ROOM_SENSOR);
			}
		}

		try {
			temp[3] = OW.getTempF(airOut.ROM);
		}
		catch (const std::out_of_range& e)
		{
			cout << "Air Out is Out of range:" << e.what() << endl;
			temp[3] = airOut.tempF;
			if (airOut.last_error_index == (airOut.read_index - 1))
			{
				//We have a consecutive error
				airOut.err_cnt++;
			}
			else
				airOut.err_cnt = 1;
			if (airOut.err_cnt > 60){
				airOut.tempF = 255;	//signal a bad reading.  We covered it for the first 60 mark it as bad now.
				errHandler(ERR_AIR_OUT_SENSOR);
			}
		}
		



		//Check validity
		for (int i = 0; i < 4; i++) {
			if ((temp[i] < 0) || (temp[i] > 150))
			{
				/*
				So I want to avoid momentary read errors by allowing 30 mis-reads for critial sensors (room, waterOut)
				*/
				switch (i)
				{
				case 0:

					if (waterIn.last_error_index == (waterIn.read_index - 1))
					{
						//So we have a consecutive error so increment err_cnt
						waterIn.err_cnt++;
					}
					pthread_mutex_lock(&wIm);
					waterIn.last_tempF = waterIn.tempF;
					waterIn.tempF = temp[0];
					pthread_mutex_unlock(&wIm);
					break;
				case 1:
					pthread_mutex_lock(&wOm);
					waterOut.last_tempF = waterOut.tempF;
					waterOut.tempF = temp[1];
					pthread_mutex_unlock(&wOm);
					break;
				case 2:
					pthread_mutex_lock(&rm);
					room.last_tempF = room.tempF;
					room.tempF = temp[2];
					pthread_mutex_unlock(&rm);
					break;
				case 3:
					pthread_mutex_lock(&aOm);
					airOut.last_tempF = airOut.tempF;
					airOut.tempF = temp[3];
					pthread_mutex_unlock(&aOm);
					break;
				}
			}
			else
			{
				switch (i)
				{
				case 0:
					pthread_mutex_lock(&wIm);
					waterIn.last_tempF = waterIn.tempF;
					waterIn.tempF = temp[0];
					pthread_mutex_unlock(&wIm);
					break;
				case 1:
					pthread_mutex_lock(&wOm);
					waterOut.last_tempF = waterOut.tempF;
					waterOut.tempF = temp[1];
					pthread_mutex_unlock(&wOm);
					break;
				case 2:
					pthread_mutex_lock(&rm);
					room.last_tempF = room.tempF;
					room.tempF = temp[2];
					pthread_mutex_unlock(&rm);
					break;
				case 3:
					pthread_mutex_lock(&aOm);
					airOut.last_tempF = airOut.tempF;
					airOut.tempF = temp[3];
					pthread_mutex_unlock(&aOm);
					break;
				}
			}
		}

		OW.startConversion();
		

	}

	

}

void errHandler(uint16_t condition)
{
	//This function will turn the machine off and set appropriate conditions.
	switch (condition)
	{
	case ERR_WATER_IN_SENSOR:
		//Not super critial.  Disables delta calculation so...
		alm |= ERR_WATER_IN_SENSOR;
		cout << "Water In Sensor Alarm" << endl;
		break;
	case ERR_WATER_OUT_SENSOR:
		//Critial Error.  Won't know if we're freezing the coil
		enabled = false;
		op_state = ALARM;
		alm |= ERR_WATER_OUT_SENSOR;
		stop_unit(op_mode);
		break;
	case ERR_ROOM_SENSOR:
		//Critical Error.  If we don't know the room temp can't do much.
		enabled = false;
		op_state = ALARM;
		alm |= ERR_ROOM_SENSOR;
		stop_unit(op_mode);
		break;
	case ERR_AIR_OUT_SENSOR:
		//Non Critial.  Take a note
		alm |= ERR_AIR_OUT_SENSOR;
		break;
	case ERR_FLOW:
		//Critial Error.  Stop Immediately!!!!!!!!!!!
		enabled = false;
		op_state = ALARM;
		alm |= ERR_FLOW;
		stop_unit(op_mode);
		break;
	case ERR_OVERFLOW:
		//Critial Error.  Stop Immediately!!!!!!!!!!!
		enabled = false;
		op_state = ALARM;
		alm |= ERR_OVERFLOW;
		stop_unit(op_mode);
		break;
	case ERR_HIGH_DELTA:
		//Critial Error.  Stop Immediately!!!!!!!!!!!!
		enabled = false;
		op_state = ALARM;
		alm |= ERR_HIGH_DELTA;
		stop_unit(op_mode);
		break;
	case ERR_LOW_WATEROUT:
		//Critical Error.  We're likely freezing the coils.
		enabled = false;
		op_state = ALARM;
		alm |= ERR_LOW_WATEROUT;
		stop_unit(op_mode);
		break;
	case ERR_HIGH_WATEROUT:
		//Critial Error.  Not enough flow.
		enabled = false;
		op_state = ALARM;
		alm |= ERR_HIGH_WATEROUT;
		stop_unit(op_mode);
		break;
	}
}

void start_unit(uint8_t _op_mode)
{
	cout << "Starting unit" << endl;
	switch (_op_mode)
	{
	case HEAT:
		setRelay(MODE_PIN, !REVERSER);
		setRelay(COMPRESSOR_PIN, ON);
		setRelay(VALVE_PIN, ON);
		op_state = WARMUP;
		sleep(10);  //In theory we are giving time for the coil to warm up prior to moving air around.
		op_state = OP_RUNNING;
		setRelay(FAN_PIN, ON);
		break;
	case COOL: //No delays in COOL mode.
		setRelay(MODE_PIN, !REVERSER);
		usleep(500000); //Give some time for the reverser valve.
		setRelay(COMPRESSOR_PIN, ON);
		setRelay(VALVE_PIN, ON);
		sleep(1); //Let the compressor stabilize before turning the fan on.
		op_state = OP_RUNNING;
		setRelay(FAN_PIN, ON);
		break;
	case FAN:
		setRelay(FAN_PIN, ON);
		op_state = OP_RUNNING;
		break;
	}
	return;
}

void stop_unit(uint8_t _op_mode)
{
	cout << "Stopping Unit" << endl;
	switch (_op_mode)
	{
	case HEAT:
		setRelay(COMPRESSOR_PIN, OFF);
		setRelay(VALVE_PIN, OFF);
		setRelay(MODE_PIN, OFF);
		op_state = COOLDOWN;
		sleep(30); //Let the fan cool the coils off
		setRelay(FAN_PIN, OFF);
		op_state = OP_STOPPED;
		break;
	default:
		setRelay(COMPRESSOR_PIN, OFF);
		setRelay(VALVE_PIN, OFF);
		setRelay(MODE_PIN, OFF);
		setRelay(FAN_PIN, OFF);
		op_state = OP_STOPPED;

	}
}


void setRelay(uint8_t relay, bool state)
{
	time_t old_compressor_time, new_compressor_time, curr_time;
	curr_time = std::time(0);
	if ((state == ON) && !enabled)
		return;
	cout << "In setRelay" << endl;
	switch (relay)
	{
	case COMPRESSOR_PIN:
		//Compressor may be changing state...
		if (compStat != state)
		{
			printf("tshoot Change Comp Relay State to %x (pre checks)\n", state);
			//We're changing states!
			if (state)
			{
				//Turning off the compressor
				digitalWrite(COMPRESSOR_PIN, state);
				compStat = state;
				compressor_last_stop = curr_time;
				fram.write(COMPRESSORLASTRUN, (uint8_t*)&old_compressor_time, 4);
				old_compressor_time = (time_t)fram.readlong(COMPRESSORRUNTIME);
				new_compressor_time = old_compressor_time + (curr_time - compressor_last_start);
				fram.write(COMPRESSORRUNTIME, (uint8_t*)&new_compressor_time, 4);

			}
			else
			{
				cout << "Turning on compressor" << endl;
				//Turning on the compressor
				compressor_last_start = std::time(0);
				//Have we ran in the last COMP_COOLDOWN seconds?
				if ((compressor_last_stop + COMP_COOLDOWN) > curr_time)
				{
					op_state = OP_WAIT;
					cout << "About to sleep for " << ((compressor_last_stop + COMP_COOLDOWN) - curr_time) << " seconds" << endl;
//  I want to make this asychronous.  Why wait?  Set mode to wait and exit.  Attempt again at correct time.
					sleep(((compressor_last_stop + COMP_COOLDOWN) - curr_time));
	//				return;
				}
				digitalWrite(COMPRESSOR_PIN, state);
				compStat = state;
				compressor_last_start = curr_time;
				fram.write(COMPRESSORLASTRUN, (uint8_t*)&curr_time, 4);

			}
		}
		break;
	case MODE_PIN:
		if (modeStat != state)
		{
			printf("tshoot Change Mode Relay State to %x\n", state);
			digitalWrite(MODE_PIN, state);
			modeStat = state;
		}
		break;
	case VALVE_PIN:
		if (fanStat != state)
		{
			printf("tshoot Change Valve Relay State to %x\n", state);
			digitalWrite(VALVE_PIN, state);
			valveStat = state;
		}
		break;
	case FAN_PIN:
		if (fanStat != state)
		{
			printf("tshoot Change FAN Relay State to %x\n", state);
			digitalWrite(FAN_PIN, state);
			fanStat = state;
		}
		break;
	case ENABLED_PIN:
		digitalWrite(ENABLED_PIN, state);
		break;
	case ALARM_PIN:
		if (almStat != state)
		{
			printf("tshoot Change Alarm Relay State to %x\n", state);
			digitalWrite(ALARM_PIN, state);
			almStat = state;
		}
		break;
	}

}

void buttonISR()
{
	uint8_t pin = buttonChip.getLastInterruptPin();
	while (buttonChip.digitalRead(pin))
		usleep(100000);


	switch (pin)
	{
	case 0:
		setPoint++;
		if (setPoint >= MAX_SETPOINT)
			setPoint = MAX_SETPOINT;
		fram.writeEnable(true);
		fram.write8(TEMP_SETPOINT, setPoint);
		fram.writeEnable(false);
//		lcd.setCursor(0, 0);
//		lcd.print(setPoint);
		break;
	case 1:
		setPoint--;
		if (setPoint <= MIN_SETPOINT)
			setPoint = MIN_SETPOINT;
		fram.writeEnable(true);
		fram.write8(TEMP_SETPOINT, setPoint);
		fram.writeEnable(false);
//		lcd.setCursor(0, 0);
//		lcd.print(setPoint);
		break;
	case 2:
		op_mode++;
		if (op_mode > 3) //We progressed past FAN so we move to DISABLED
			op_mode = 0;
		mode_change = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		fram.writeEnable(true);
		fram.write8(LASTMODE, op_mode);
		fram.writeEnable(false);
//		lcd.setCursor(10, 0);
//		lcd.print(op_mode);
		break;
	case 3:
		if (enabled)
			enabled = false;
		else
			enabled = true;
		fram.writeEnable(true);
		fram.write8(F_ENABLED, enabled);
		fram.writeEnable(false);
		buttonChip.digitalWrite(7, enabled); //Visual indicator
		setRelay(ENABLED_PIN, !enabled); //Relays are low = on so have to use opposite of enabled.
		printf("Enable change to %x \n", enabled);

		break;
	case 4:
		break;
	default:
		break;
	}

}

void flowCounter()
{
	flowcounts++;
	if (flowcounts > 189)
	{
		flowcounts = 0;
		deciGallons++;
	}
}

void setDisplays()
{
	uint8_t old_setpoint = 0;
	float old_temp[3] = { 0, 0, 0 };
	float temp;
	struct tm * compt;
	
	Smith_7segment redseg = Smith_7segment();
	Smith_7segment whiteseg = Smith_7segment();
	Smith_7segment blueseg = Smith_7segment();
	Smith_7segment greenseg = Smith_7segment();
	Smith_AlphaNum4 modeseg = Smith_AlphaNum4();

	modeseg.begin(0x72, 0);
	whiteseg.begin(0x73, 0);
	redseg.begin(0x74, 0);
	blueseg.begin(0x75, 0);
	greenseg.begin(0x76, 0);

	
	redseg.setBrightness(5);
	whiteseg.setBrightness(15);
	blueseg.setBrightness(5);
	greenseg.setBrightness(5);
	modeseg.setBrightness(1);

	

	while (1)
	{
		if (sigint)
		{
			break;
		}
//		cout << "while" << endl;
		pthread_mutex_lock(&wIm);
		temp = waterIn.tempF;
		pthread_mutex_unlock(&wIm);

		if (old_temp[0] != temp)
		{
			blueseg.print(temp);
			blueseg.writeDisplay();
			old_temp[0] = temp;
		}

		pthread_mutex_lock(&wOm);
		temp = waterOut.tempF;
		pthread_mutex_unlock(&wOm);
		if (old_temp[1] != temp)
		{
			redseg.print(temp);
			redseg.writeDisplay();
			old_temp[1] = temp;
		}

		pthread_mutex_lock(&rm);
		temp = room.tempF;
		pthread_mutex_unlock(&rm);
		if (old_temp[2] != temp)
		{
			greenseg.print(temp);
			greenseg.writeDisplay();
			old_temp[2] = temp;
		}

		pthread_mutex_lock(&aOm);
		old_temp[3] = airOut.tempF;
		pthread_mutex_unlock(&aOm);

		if (old_setpoint != setPoint)
		{
			whiteseg.print((int)setPoint);
			whiteseg.writeDisplay();
			old_setpoint = setPoint;
		}



		
		usleep(500000);


		switch (op_mode)
		{
		case DISABLED:
			modeseg.writeDigitAscii(0, ' ', 0);
			modeseg.writeDigitAscii(1, 'O', 0);
			modeseg.writeDigitAscii(2, 'F', 0);
			modeseg.writeDigitAscii(3, 'F', 0);
			break;
		case FAN:
			modeseg.writeDigitAscii(0, ' ', 0);
			modeseg.writeDigitAscii(1, 'F', 0);
			modeseg.writeDigitAscii(2, 'A', 0);
			modeseg.writeDigitAscii(3, 'N', 0);
			break;
		case COOL:
			modeseg.writeDigitAscii(0, 'C', 0);
			modeseg.writeDigitAscii(1, 'O', 0);
			modeseg.writeDigitAscii(2, 'O', 0);
			modeseg.writeDigitAscii(3, 'L', 0);
			break;
		case HEAT:
			modeseg.writeDigitAscii(0, 'H', 0);
			modeseg.writeDigitAscii(1, 'E', 0);
			modeseg.writeDigitAscii(2, 'A', 0);
			modeseg.writeDigitAscii(3, 'T', 0);	
		}
		rawtime = time(0);
		now = localtime(&rawtime);
		snprintf(lcdpanel[3], 21, "TIME IS %d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
		compt = localtime((const time_t *)&compressor_last_stop);
		snprintf(lcdpanel[1], 21, "COMP Last %d:%02d:%02d", compt->tm_hour, compt->tm_min, compt->tm_sec);
//		printf("tshooting rawtime is %d compt is %d \n", rawtime, compressor_last_stop);

		switch (op_state)
		{
		case OP_STOPPED:
			snprintf(lcdpanel[0], 21, "IDLE ROOM TEMP %3.1f ", old_temp[2]);

			snprintf(lcdpanel[2], 21, "                    ");

			break;
		case OP_RUNNING:
			snprintf(lcdpanel[0], 21, "RUNNING I:%3.1f O:%3.1f ", old_temp[2], old_temp[3]);
			snprintf(lcdpanel[1], 21, "WATER   i:%3.1F O:%3.1F ", old_temp[0], old_temp[1]);
			snprintf(lcdpanel[2], 21, "DELTA:%3.1F GPM:      ", abs(old_temp[1]-old_temp[0]));
			snprintf(lcdpanel[3], 21, "RUN TIME HH:MM:SS   ");
			break;
		case OP_WAIT:
			snprintf(lcdpanel[0], 21, "WAIT FOR COMPRESSOR ");
			snprintf(lcdpanel[1], 21, "                    ");
			snprintf(lcdpanel[2], 21, "                    ");
			snprintf(lcdpanel[3], 21, "                    ");
			break;
		case COOLDOWN:
			snprintf(lcdpanel[0], 21, "COOLDOWN            ");
			snprintf(lcdpanel[1], 21, "                    ");
			snprintf(lcdpanel[2], 21, "                    ");
			snprintf(lcdpanel[3], 21, "                    ");
			break;
		case ALARM:
			snprintf(lcdpanel[0], 21, "ALARM               ");
			snprintf(lcdpanel[1], 21, "                    ");
			snprintf(lcdpanel[2], 21, "                    ");
			snprintf(lcdpanel[3], 21, "                    ");
			break;

		}
		for (int i = 0; i < 4; i++)
		{
//			lcd.clear();
			lcd.setCursor(0, i);
			lcd.print(lcdpanel[i]);
		}


		modeseg.writeDisplay();
//		modeseg.writeDigitAscii(0, 'H', 0);
//		modeseg.writeDigitAscii(1, '3', 0);
//		modeseg.writeDigitAscii(2, '2', 0);
//		modeseg.writeDigitAscii(3, '1', 0);
		
//		modeseg.writeDisplay();
	}
	redseg.println(1111);
	whiteseg.println(1111);
	blueseg.println(1111);
	greenseg.println(1111);
	modeseg.writeDigitAscii(0, 'E', 0);
	modeseg.writeDigitAscii(1, 'X', 0);
	modeseg.writeDigitAscii(2, 'I', 0);
	modeseg.writeDigitAscii(3, 'T', 0);

	redseg.setBrightness(1);
	whiteseg.setBrightness(1);
	blueseg.setBrightness(0);
	greenseg.setBrightness(0);
	modeseg.setBrightness(0);

	redseg.writeDisplay();
	whiteseg.writeDisplay();
	blueseg.writeDisplay();
	greenseg.writeDisplay();
	modeseg.writeDisplay();
	lcd.clear();

}

void monitor()
{
	//This thread will be used to check things like the overflow state, etc.



	while (1)
	{
		switch (op_state)
		{
		case OP_RUNNING:
		case WARMUP:
			//If the unit is on monitor:
			//flow switch state
			//condensate overflow state
			//water output temp
			//water delta
			break;
		}
	}
}