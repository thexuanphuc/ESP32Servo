/*
 * ESP32Tone.cpp
 *
 *  Created on: Sep 23, 2018
 *      Author: hephaestus
 */


#include "ESP32Tone.h"

void tone(int pin,unsigned int frequency){
	ESP32PWM* chan = pwmFactory(pin);
	if (chan == NULL) {
		chan = new ESP32PWM();
		chan->setup(1000, 10); // 1KHz 8 bit
		chan->attachPin(pin); // This adds the PWM instance to the factory list
		//Serial.println("Attaching AnalogWrite : "+String(APin)+" on PWM "+String(chan->getChannel()));
	}
	chan->writeTone(frequency);// update the time base of the PWM
}

void tone(int pin, unsigned int frequency, unsigned long duration){
	tone(pin,frequency);
	delay(duration);
	noTone(pin);
}

void noTone(int pin){
	ESP32PWM* chan = pwmFactory(pin);
	if (chan != NULL) {
		if(chan->attached())
			chan->detachPin(pin);
	}
}
