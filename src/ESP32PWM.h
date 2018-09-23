/*
 * ESP32PWM.h
 *
 *  Created on: Sep 22, 2018
 *      Author: hephaestus
 */

#ifndef LIBRARIES_ESP32SERVO_SRC_ESP32PWM_H_
#define LIBRARIES_ESP32SERVO_SRC_ESP32PWM_H_
#include "esp32-hal-ledc.h"
#define NUM_PWM 16
#define USABLE_ESP32_PWM (NUM_PWM-1)
#include <cstdint>
#include "esp32-hal-ledc.h"
#include "Arduino.h"
class ESP32PWM {
private:

public:
	int pin;

	ESP32PWM();
	virtual ~ESP32PWM();
	int pwmChannel = 0;                         // channel number for this servo
	void attach(int pin);
	void detach();
	//channel 0-15 resolution 1-16bits freq limits depend on resolution9
//	double      setup( double freq, uint8_t resolution_bits);
//	void        write( uint32_t duty);
//	double      writeTone( double freq);
//	double      writeNote( note_t note, uint8_t octave);
//	uint32_t    read();
//	double      readFreq();
//	void        attachPin(uint8_t pin);
//	void        detachPin(uint8_t pin);
};
ESP32PWM* pwmFactory(int pin);

#endif /* LIBRARIES_ESP32SERVO_SRC_ESP32PWM_H_ */
