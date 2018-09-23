/*
 * analogWrite.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: Harry-Laptop
 */

#include "analogWrite.h"
#include "ESP32PWM.h"


void analogWrite(uint8_t APin, uint16_t AValue) {
	ESP32PWM* chan = pwmFactory(APin);
	if (chan == NULL) {
		chan = new ESP32PWM();
		chan->pin = APin;
		chan->setup( 1000, 8); // 1KHz 8 bit
		chan->attachPin(APin);
	}
	chan->write( AValue);
//    Serial.print( "ledcWrite: " ); Serial.print(  CESP32PWMPinMap[ APin ] - 1 ); Serial.print( " " ); Serial.println( AValue );
}
