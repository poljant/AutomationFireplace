/*
 * FireplaceController.cpp
 *
 *   Copyright (c) 2018. All rights reserved.
 *
 *      Author: Jan Trzciński  <poljant@post.pl>
 */

#include "../AutomationFireplace/FireplaceController.h"

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Relay.h>
#include "../AutomationFireplace/PWM.h"
#include "../AutomationFireplace/RF2260.h"

const char *codOn1 = ("000101010001010111000011"); //attachment code
const char *codOff1 = ("000101010001010111001100"); //disabling code
const char *codOn2 = ("000101010001010100110011"); //attachment code
const char *codOff2 = ("000101010001010100111100"); //disabling code
const char *codOn3 = ("010000010001010111000011"); //attachment code
const char *codOff3 = ("010000010001010111001100"); //disabling code

// pins definition
#define pin_relay1 D8
#define pin_relay2 D0
#define pin_relay3 D7  //optional switch or pwm3
#define pin_pwm1 D6
#define pin_pwm2 D5
//#define pin_pwm3 D7	//optional pwm3 or relay3

//  I2C
#define SLC D2
#define SDA D1
//PT100
//#define pomiar A0

// pin connect RF
#define RFpin D4
unsigned int lH = 200; // duration (us) state H for RFpin
// pin OneWire
#define pinOW D3
// configuration 1-Wire
OneWire oneWire(pinOW);
DallasTemperature sensors(&oneWire);

FireplaceController::FireplaceController() {

	relay1.begin(pin_relay1);
	relay2.begin(pin_relay2);
	relay3.begin(pin_relay3);

	pwm.setFreq(PWMfreq);
	pwm.init(pin_pwm1, pin_pwm2);
	pwm.start();
	rf1.setRF(RFpin, lH, codOn1, codOff1);
	rf2.setRF(RFpin, lH, codOn2, codOff2);
	rf3.setRF(RFpin, lH, codOn3, codOff3);
	timecurrent = 0;
	storey = 0;
	start_automation = false;

}

FireplaceController::~FireplaceController() {
	// TODO Auto-generated destructor stub
}

void FireplaceController::setAuto(void) {
	bmode = true;
}
void FireplaceController::setManual(void) {
	bmode = false;
}
void FireplaceController::begin(void) {
	FireplaceController();
}

void FireplaceController::working(void) {
//	int p = 0;

	if (millis() >= timecurrent) {
		if (!breadTemp) {
			timecurrent = millis() + 850;
			readTemp(false);
			breadTemp = true;
		} else {
			timecurrent = millis() + timedelay;
			readTemp(true);
			breadTemp = false;
		}
		//readTemp(bread);
	}
	if (bmode) {
		//readTemp();
		if (temp_alarm <= temp_in_box) {
			alarm = true;
			relay2.setOn();
		} else {
			alarm = false;
			relay2.setOff();
		}

//jeśli temperatura rośnie od minimalnej (gdy rozpalono w kominku)
		if ((temp_in_box >= temp_on1) and !start_automation and storey == 0) {
			start_automation = true; //ustaw start procesu grzania
			storey = 1;	//ustw poziom 1
//			relay1.setOn();
			relay3.setOn();
			if (rf1.readRF() == 0) { // send RF switching signal when rf1 is turned off
				rf1.sendOn();
			};
		}
		if (start_automation) {
//jeśli cykl grzania już trwa i jest poziom 1
// gdy temperatura spada poniżej poziomu 1
			if ((temp_in_box <= temp_off1) and storey == 1) {
				start_automation = false;
				storey = 0;
//				setFans(storey);
//			relay1.setOff();
				relay3.setOff();
				if (rf1.readRF() == 1) { // send an RF off signal when rf1 is on
					rf1.sendOff();
				}
			}

// gdy temperatura przekroczyła poziom 2
			if ((temp_in_box >= temp_on2) and storey == 1) {
				storey = 2;
				relay1.setOn();
				if (rf2.readRF() == 0) { // send RF switching signal when rf2 is turned off
					rf2.sendOn();
				};
// gdy temperatura spada i jest poziom 2
			}
			if ((temp_in_box <= temp_off2) and storey == 2) {
				storey = 1;
				relay1.setOff();
				if (rf2.readRF() == 1) { // send RF switching signal when rf2 is turned on
					rf2.sendOff();
				};
			}
//gdy temperatura przekroczyła poziom 3
			if ((temp_in_box >= temp_on3) and storey == 2) {
				storey = 3;
				if (rf3.readRF() == 0) { // send RF switching signal when rf3 is turned off
					rf3.sendOn();
				};
// gdy temperatura spada i jest poziom 3
			} //else
			if ((temp_in_box <= temp_off3) and storey == 3) {
				storey = 2;
				if (rf3.readRF() == 1) { // send an RF off signal when rf3 is on
					rf3.sendOff();
				};
			}
//ustaw obroty wentylatorów zależnie od poziomy
			setFans(storey);
		} //end star_automation
	} //end bmode

}
//}
void FireplaceController::readTemp(bool breadx) {
	if (!breadx) {
		sensors.requestTemperatures();  // start reading the temperature sensors
		//delay(850);
	} else {
		temp_current = sensors.getTempCByIndex(0);  //read the temperature at ºC
		if (temp_current == -127 or temp_current == 85) {
			timecurrent = millis() + 1000;
			return;
		}  // if reading error
		temp_in_box = temp_current;
		if (temp_in_box > temp_max)
			temp_max = temp_in_box;
	}
}

float FireplaceController::readTempIn(void) {
	return temp_in_box;
}
bool FireplaceController::readMode(void) {
	return bmode;
}
int FireplaceController::percent2duration(long int v) {
	return (int) map(v, 0, 100, 0, 1023);
}
int FireplaceController::duration2percent(long int v) {
	return (int) map(v, 0, 1023, 0, 100);
}

int FireplaceController::temp2duration(long int v) {
	if (v < temp_off)
		return 0;
	if (v > temp_on2)
		v = temp_on2;
	return (int) map(v, temp_off, temp_on2, 450, 1023);//450 na 600
}
void FireplaceController::setFans(int i) {
	if (start_automation and (temp_off<=temp_in_box) and i > 0) {
		if (program == 1) {
			fanx = temp2duration(temp_in_box);

			if (fanx < 0)
				fanx = 0;
			if (i > 0) {
//			fanx = temp2duration(temp_in_box);
				fan1 = duration2percent(fanx);
				fan2 = duration2percent(fanx);
			} else {
				fan1 = 0;
				fan2 = 0;
			}
		} else {
			if (i > 3)
				i = 3;
			if (i < 1)
				i = 1;
			fan1 = Fan1Speed[i];
			fan2 = Fan2Speed[i];
		}
		if (fan1 > 0) {
			pwm.write(0, percent2duration(fan1));
		} else {
			pwm.write(0, 0);
		}
		if (fan2 > 0) {
			pwm.write(1, percent2duration(fan2));
		} else {
			pwm.write(1, 0);
		}
	} else {
		pwm.write(0, 0);
		pwm.write(1, 0);
		fan1 = 0;
		fan2 = 0;
	}
}
