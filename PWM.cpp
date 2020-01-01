/*
 * PWM.cpp
 *
 *  Created on: 17.12.2017
 *      Author: jant
 */

#include "../AutomationFireplace/PWM.h"

#include <Arduino.h>

PWM::PWM() {
	channels = 3;
	freq = 1000;
}

PWM::~PWM() {

}
void PWM::init(int pin1, int pin2, int pin3) {
	channels = 3;
	channel[0][0] = pin1;
	channel[1][0] = pin2;
	channel[2][0] = pin3;
	set();
}
void PWM::init(int pin1, int pin2) {
	channels = 2;
	channel[0][0] = pin1;
	channel[1][0] = pin2;
	set();
}
void PWM::init(int pin1) {
	channels = 1;
	channel[0][0] = pin1;
	set();
}
void PWM::set() {

	for (int i = 0; i < channels; i++) {
		pinMode(channel[i][0], OUTPUT);
		analogWrite(channel[i][0], LOW);
		channel[i][1] = 0;
	}

}

void PWM::setFreq(int f) {
	freq = f;
	analogWriteFreq(freq);
}
void PWM::write(int ch, int v) {
	if (ch > channels)
		return;
	channel[ch][1] = v;
	analogWrite(channel[ch][0], v);
}
void PWM::start(void) {
	for (int i = 0; i < channels; i++) {
		analogWrite(channel[i][0], channel[i][1]);
	}
}
int PWM::read(int ch) {
	if (ch > channels)
		return 0;
	return analogRead(channel[ch][1]);
}
int PWM::readFreq(void) {
	return freq;
}
