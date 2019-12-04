/*
 * FireplaceController.cpp
 *
 *   Copyright (c) 2018. All rights reserved.
 *
 *      Author: Jan Trzciński  <poljant@post.pl>
 */


#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Relay.h>
#include "FireplaceController.h"
#include "PWM.h"
#include "RF2260.h"

const char* codOn1 =  ("000101010001010111000011"); //attachment code
const char* codOff1 = ("000101010001010111001100"); //disabling code
const char* codOn2 =  ("000101010001010100110011"); //attachment code
const char* codOff2 = ("000101010001010100111100"); //disabling code
const char* codOn3 =  ("010000010001010111000011"); //attachment code
const char* codOff3 = ("010000010001010111001100"); //disabling code

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

FireplaceController
::FireplaceController() {

	relay1.begin(pin_relay1);
	relay2.begin(pin_relay2);
	relay3.begin(pin_relay3);

	pwm.setFreq(PWMfreq);
	pwm.init(pin_pwm1, pin_pwm2);
	pwm.start();
	rf1.setRF(RFpin,lH, codOn1, codOff1);
	rf2.setRF(RFpin,lH, codOn2, codOff2);
	rf3.setRF(RFpin,lH, codOn3, codOff3);

}

FireplaceController::~FireplaceController() {
	// TODO Auto-generated destructor stub
}

void FireplaceController::setAuto(void){
	bmode = true;
}
void FireplaceController::setManual(void){
	bmode = false;
}
void FireplaceController::begin(void){
	FireplaceController();
}

void FireplaceController::working(void){
int p=0;

	if(bmode){
		readTemp();

		if(temp_in_box>temp_on1)p=0;
		if(temp_alarm<=temp_in_box){
			p=3;
			if (alarm){
				relay2.setOn();
			}else{
				relay2.setOff();
			}
		}else{
			relay2.setOff();
			alarm= true;
		}

		if (temp_in_box >= temp_on1){
			p=1;
			relay3.setOn();
			if (rf1.readRF()==0){ // send RF switching signal when rf1 is turned off
			rf1.sendOn();
			};

		}
		else if (temp_in_box <= temp_off1) {
			p=0;
			relay3.setOff();
			if(rf1.readRF()==1){ // send an RF off signal when rf1 is on
			rf1.sendOff();
			}

		}

			if (temp_in_box >= temp_on2){
				p=2;
				relay1.setOn();
				if (rf2.readRF()==0){ // send RF switching signal when rf2 is turned off
				rf2.sendOn();
				};

			}
			else if (temp_in_box <= temp_off2) {
				p=1;
				relay1.setOff();
				if (rf2.readRF()==1){ // send RF switching signal when rf2 is turned on
				rf2.sendOff();
				};

			}
			if (temp_in_box >= temp_on3){
				p=3;
				//	relay2.setOn();
				if (rf3.readRF()==0){ // send RF switching signal when rf3 is turned off
				rf3.sendOn();
				};

			}else if (temp_in_box <= temp_off3) {
					p=2;
				//	relay2.setOff();
				if(rf3.readRF()==1){ // send an RF off signal when rf3 is on
				rf3.sendOff();
				};
			}
			if(p>=0){
			setFans(p);
	}
		} //end bmode



	}
//}
void FireplaceController::readTemp(void){

	 sensors.requestTemperatures();  // start reading the temperature sensors
	 delay(1000);
	 temp_current = sensors.getTempCByIndex(0);//read the temperature at ºC
	 if (temp_current ==-127 or temp_current == 85) return; // if reading error
	 temp_in_box = temp_current;
	 //	 temp_in_box = 55;
	 if (temp_in_box>temp_max) temp_max=temp_in_box;
}

float FireplaceController::readTempIn(void){
	return temp_in_box;
}
bool FireplaceController::readMode(void){
	return bmode;
}
int FireplaceController::percent2duration(long int v){
	return (int)map(v,0,100,0,1023);
}
int FireplaceController::duration2percent(long int v){
	return (int)map(v,0,1023,0,100);
}

int FireplaceController::temp2duration(long int v){
	if(v<temp_on1) return 0;
	if(v>100) v=100;
	return (int)map(v,temp_on1,100,450,1023);
}
void FireplaceController::setFans( int i){
	if (program==1){
	fanx = temp2duration(temp_in_box);
	if(fanx<0)fanx=0;
	fan1 = duration2percent(fanx);
	fan2 = duration2percent(fanx);
	}else{
		if (i>3) i=3;
		if (i<0) i=0;
	fan1 = Fan1Speed[i];
	fan2 = Fan2Speed[i];
	}
	pwm.write(0, percent2duration(fan1));
	pwm.write(1, percent2duration(fan2));
}
