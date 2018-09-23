/*
 * ESP32PWM.h
 *
 *  Created on: Sep 22, 2018
 *      Author: hephaestus
 */

#ifndef LIBRARIES_ESP32SERVO_SRC_ESP32PWM_H_
#define LIBRARIES_ESP32SERVO_SRC_ESP32PWM_H_
#include "esp32-hal-ledc.h"
#define NUM_WPM 16
#include <cstdint>
#include "esp32-hal-ledc.h"
#include "Arduino.h"
class ESP32PWM {

public:

	int pin;

	ESP32PWM();
	virtual ~ESP32PWM();
	int pwmChannel = 0;                         // channel number for this servo
	void attach(int pin);
	void detach();
};
ESP32PWM* pwmFactory(int pin);

#endif /* LIBRARIES_ESP32SERVO_SRC_ESP32PWM_H_ */
