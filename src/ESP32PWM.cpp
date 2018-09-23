/*
 * ESP32PWM.cpp
 *
 *  Created on: Sep 22, 2018
 *      Author: hephaestus
 */

#include <ESP32PWM.h>
#include "esp32-hal-ledc.h"
// initialize the class variable ServoCount
static int ServoCount = -1;              // the total number of attached servos
static ESP32PWM * ChannelUsed[NUM_PWM]; // used to track whether a channel is in service
// The ChannelUsed array elements are 0 if never used, 1 if in use, and -1 if used and disposed
// (i.e., available for reuse)

ESP32PWM::ESP32PWM() {
	if (ServoCount == -1) {
		for (int i = 0; i < NUM_PWM; i++)
			ChannelUsed[i] = NULL; // load invalid data into the storage array of pin mapping
		ServoCount = 1; // 0th channel does not work with the PWM system

	}
	if (ServoCount == NUM_PWM) {
		return;
	}
	pwmChannel = -1;
	pin = -1;
	for (int i = ServoCount; i < NUM_PWM; i++) {
		if (ChannelUsed[i] == NULL) {
			ChannelUsed[i] = this;
			pwmChannel = i;
			ServoCount++;
			Serial.println("PWM channel requested " + String(i));
			return;
		}
	}
	Serial.println("ERROR All PWM channels requested! " + String(ServoCount));

}

ESP32PWM::~ESP32PWM() {
	// TODO Auto-generated destructor stub
}

void ESP32PWM::detach() {
	ChannelUsed[pwmChannel] = NULL;
	pwmChannel = -1;
	pin = -1;

}

void ESP32PWM::attach(int p) {
	pin = p;
}

double ESP32PWM::setup(double freq, uint8_t resolution_bits) {
	return ledcSetup(pwmChannel, freq, resolution_bits);
}
void ESP32PWM::write(uint32_t duty) {
	return ledcWrite(pwmChannel, duty);
}
double ESP32PWM::writeTone(double freq) {
	return ledcWriteTone(pwmChannel, freq);
}
double ESP32PWM::writeNote(note_t note, uint8_t octave) {
	return ledcWriteNote(pwmChannel, note, octave);
}
uint32_t ESP32PWM::read() {
	return ledcRead(pwmChannel);
}
double ESP32PWM::readFreq() {
	return ledcReadFreq(pwmChannel);
}
void ESP32PWM::attachPin(uint8_t pin) {
	attach(pin);
	return ledcAttachPin(pin,pwmChannel);
}
void ESP32PWM::detachPin(uint8_t pin) {
	detach() ;
	return ledcDetachPin(pin);
}


ESP32PWM* pwmFactory(int pin) {
	for (int i = 0; i < NUM_PWM; i++)
		if (ChannelUsed[i] != NULL) {
			if (ChannelUsed[i]->pin == pin)
				return ChannelUsed[i];
		}
	return NULL;
}
