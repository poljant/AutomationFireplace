/*
 * RF2260.cpp
 *
 * Copyright (c) 2016 - 2018. All rights reserved.
 *
 *      Author: Jan Trzciński  <poljant@post.pl>
 *
 *      Klasa RF2260 umożliwia nadawanie kodów dla układu z serii 2260.i podobnych.
 *      Układ ten stosowany jest w tanich i popularnych wyłacznikach 220V.
 *      Częstotliwość nadawania to 433MHz  lub 896MHz.
 *      Do uruchomienia układu potrzebny jest nadajnik RF na 433MHz lub 866MHz
 */
#include "RF2260.h"

#include <Arduino.h>

#define pinRF D0	//pin to which we connect the RF transmitter

//extern char* codOn; // = "000101010001010111000011"; //attachment code
//extern char* codOff; // = "000101010001010111001100"; //disabling code
unsigned int lh = 200; // duration (in us) of high state on RF opinions
unsigned int replay = 10; // how many times to repeat the code
//namespace RF2260 {
 RF2260::RF2260()
 {
	 RFpin = pinRF;
//	 pinMode(RFpin, OUTPUT);
	 // RF2260::setRF(pinRF, lh, codOn, codOff);
 }

 RF2260::~RF2260() {
	// TODO Auto-generated destructor stub

 }
 void RF2260::setCodOn(char* cod){
	codOn = cod;
 }
 void RF2260::setCodOff(char* cod){
	codOff = cod;
 }
// send 0 to the RFpin bus
//   _
//  | |___
 void RF2260::send0(void) {
	digitalWrite(RFpin, HIGH);
	delayMicroseconds(lH);
	digitalWrite(RFpin, LOW);
	delayMicroseconds(lH * 3);
 }
// send 1 to the RFpin bus
//   ___
//  |   |_
 void RF2260::send1(void) {
	digitalWrite(RFpin, HIGH);
	delayMicroseconds((lH * 3));
	digitalWrite(RFpin, LOW);
	delayMicroseconds(lH);
 }
// send a sync pulse
//   _
// _| |________________
 void RF2260::sendSync(void){
	RF2260::send0();
	delay(5 * lH/1000);
	delay(5 * lH/1000);
 }
 void RF2260::sendRF(char* cod){
	for (unsigned int j = 0; j < replay; j++){
		unsigned int i = 0;
		while (cod[i] !='\0'){
			switch(cod[i]) {
			case '0':
				RF2260::send0();
				break;
			case '1':
				RF2260::send1();
				break;
			}
			i++;
		}
		RF2260::sendSync();
	}
 }
 void RF2260::sendOff(void){

	 RF2260::sendRF(codOff);
	 RF=0;
 }
 void RF2260::sendOn(void){

	 RF2260::sendRF(codOn);
	 RF=1;
 }
//read whether RF is On
 unsigned int RF2260::readRF(){
	return RF;
 }
 //set the RF status
 void RF2260::writeRF(unsigned int rfx){
	 RF=rfx;
 }
 void RF2260::setRF(unsigned int pin, unsigned int l, char* On, char* Off){
	RFpin = pin; // set the RF connection pin
	pinMode(RFpin, OUTPUT);
	lH = l; // duration (us) of the high state on the RF pin
	RF2260::setCodOn(On);
	RF2260::setCodOff(Off);
	RF = 0; // 1 - ON , 0 - OFF

 }

//} /* namespace RF2260 */
