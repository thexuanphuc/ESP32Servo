/*
 * ESP32PWM.cpp
 *
 *  Created on: Sep 22, 2018
 *      Author: hephaestus
 */

#include <ESP32PWM.h>
#include "esp32-hal-ledc.h"
#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp32-hal-matrix.h"
#include "soc/dport_reg.h"
#include "soc/ledc_reg.h"
#include "soc/ledc_struct.h"

#if CONFIG_DISABLE_HAL_LOCKS
#define LEDC_MUTEX_LOCK()
#define LEDC_MUTEX_UNLOCK()
#else
#define LEDC_MUTEX_LOCK()    do {} while (xSemaphoreTake(_ledc_sys_lock, portMAX_DELAY) != pdPASS)
#define LEDC_MUTEX_UNLOCK()  xSemaphoreGive(_ledc_sys_lock)
extern xSemaphoreHandle _ledc_sys_lock;
#endif
#define LEDC_CHAN(g,c) LEDC.channel_group[(g)].channel[(c)]
#define LEDC_TIMER(g,t) LEDC.timer_group[(g)].timer[(t)]

// initialize the class variable ServoCount
int ESP32PWM::PWMCount = -1;              // the total number of attached servos
ESP32PWM * ESP32PWM::ChannelUsed[NUM_PWM]; // used to track whether a channel is in service
long ESP32PWM::timerFreqSet[4] = { -1, -1, -1, -1 };
int ESP32PWM::timerCount[4] = { 0, 0, 0, 0 };
// The ChannelUsed array elements are 0 if never used, 1 if in use, and -1 if used and disposed
// (i.e., available for reuse)

ESP32PWM::ESP32PWM() {
	resolutionBits = 8;
	pwmChannel = -1;
	pin = -1;
	myFreq = -1;
	if (PWMCount == -1) {
		for (int i = 0; i < NUM_PWM; i++)
			ChannelUsed[i] = NULL; // load invalid data into the storage array of pin mapping
		PWMCount = PWM_BASE_INDEX; // 0th channel does not work with the PWM system
	}
}

ESP32PWM::~ESP32PWM() {
	// TODO Auto-generated destructor stub
}
void ESP32PWM::_ledcSetupTimer(uint8_t chan, uint32_t div_num, uint8_t bit_num, bool apb_clk)
{
    uint8_t group=(chan/8), timer=((chan/2)%4);

    LEDC_MUTEX_LOCK();
    LEDC_TIMER(group, timer).conf.clock_divider = div_num;//18 bit (10.8) This register is used to configure parameter for divider in timer the least significant eight bits represent the decimal part.
    LEDC_TIMER(group, timer).conf.duty_resolution = bit_num;//5 bit This register controls the range of the counter in timer. the counter range is [0 2**bit_num] the max bit width for counter is 20.
    LEDC_TIMER(group, timer).conf.tick_sel = apb_clk;//apb clock
    if(group) {
        LEDC_TIMER(group, timer).conf.low_speed_update = 1;//This bit is only useful for low speed timer channels, reserved for high speed timers
    }
    LEDC_TIMER(group, timer).conf.pause = 0;
    LEDC_TIMER(group, timer).conf.rst = 1;//This bit is used to reset timer the counter will be 0 after reset.
    LEDC_TIMER(group, timer).conf.rst = 0;
    LEDC_MUTEX_UNLOCK();
}

double ESP32PWM::_ledcSetupTimerFreq(uint8_t chan, double freq, uint8_t bit_num)
{
    uint64_t clk_freq = APB_CLK_FREQ;
    clk_freq <<= 8;//div_num is 8 bit decimal
    uint32_t div_num = (clk_freq >> bit_num) / freq;
    bool apb_clk = true;
    if(div_num > LEDC_DIV_NUM_HSTIMER0_V) {
        clk_freq /= 80;
        div_num = (clk_freq >> bit_num) / freq;
        if(div_num > LEDC_DIV_NUM_HSTIMER0_V) {
            div_num = LEDC_DIV_NUM_HSTIMER0_V;//lowest clock possible
        }
        apb_clk = false;
    } else if(div_num < 256) {
        div_num = 256;//highest clock possible
    }
    _ledcSetupTimer(chan, div_num, bit_num, apb_clk);
    //log_i("Fin: %f, Fclk: %uMhz, bits: %u, DIV: %u, Fout: %f",
    //        freq, apb_clk?80:1, bit_num, div_num, (clk_freq >> bit_num) / (double)div_num);
    return (clk_freq >> bit_num) / (double)div_num;
}


int ESP32PWM::timerAndIndexToChannel(int timerNum, int index) {
	int localIndex = 0;
	for (int j = 0; j < NUM_PWM; j++) {
		if (((j / 2) % 4) == timerNum) {
			if (localIndex == index) {
				return j;
			}
			localIndex++;
		}
	}
	return -1;
}
int ESP32PWM::allocatenext(double freq) {

	long freqlocal = (long) freq;
	if (pwmChannel < 0) {
		for (int i = 0; i < 4; i++) {
			bool freqAllocated = ((timerFreqSet[i] == freqlocal)
					|| (timerFreqSet[i] == -1));
			if (freqAllocated && timerCount[i] < 4) {
				if (timerFreqSet[i] == -1) {
					//Serial.println("Starting timer "+String(i)+" at freq "+String(freq));
					timerFreqSet[i] = freqlocal;
				}
				//Serial.println("Free channel timer "+String(i)+" at freq "+String(freq)+" remaining "+String(4-timerCount[i]));

				timerNum = i;
				int myTimerNumber = timerAndIndexToChannel(timerNum,
						timerCount[timerNum]);
				if (myTimerNumber > 0) {
					pwmChannel = myTimerNumber;
					Serial.println(
							"PWM on ledcwrite channel " + String(pwmChannel)
									+ " using timer " + String(timerNum)
									+ " to freq " + String(freq));
					ChannelUsed[pwmChannel] = this;
					timerCount[timerNum]++;
					PWMCount++;
					myFreq = freq;
					return pwmChannel;
				}
			} else {
//				if(timerFreqSet[i]>0)
//					Serial.println("Timer freq mismatch target="+String(freq)+" on tmer "+String(i)+" was "+String(timerFreqSet[i]));
//				else
//					Serial.println("Timer out of channels target="+String(freq)+" on tmer "+String(i)+" was "+String(timerCount[i]));
			}
		}
	} else {
		return pwmChannel;
	}
	Serial.println("ERROR All PWM channels requested! " + String(freq));
	while (1)
		;
}
void ESP32PWM::deallocate() {
	if(pwmChannel<0)
		return;
	Serial.println("PWM deallocating LEDc #" + String(pwmChannel));
	timerCount[getTimer()]--;
	if (timerCount[getTimer()] == 0) {
		timerFreqSet[getTimer()] = -1; // last pwn closed out
	}
	timerNum = -1;
	attachedState = false;
	ChannelUsed[pwmChannel] = NULL;
	pwmChannel = -1;
	PWMCount--;

}


int ESP32PWM::getChannel() {
	if (pwmChannel < 0) {
		Serial.println("FAIL! must setup() before using get channel!");
	}
	return pwmChannel;
}

double ESP32PWM::setup(double freq, uint8_t resolution_bits) {
	checkFrequencyForSideEffects(freq);

	resolutionBits = resolution_bits;
	if (attached()) {
		ledcDetachPin(pin);
		double val = ledcSetup(getChannel(), freq, resolution_bits);
		attachPin(pin);
		return val;
	}
	return ledcSetup(getChannel(), freq, resolution_bits);
}
float ESP32PWM::getDutyScaled() {
	return mapf((float) myDuty, 0, (float) ((1 << resolutionBits) - 1), 0.0,
			1.0);
}
void ESP32PWM::writeScaled(float duty) {
	write(mapf(duty, 0.0, 1.0, 0, (float) ((1 << resolutionBits) - 1)));
}
void ESP32PWM::write(uint32_t duty) {
	myDuty = duty;
	ledcWrite(getChannel(), duty);
}
void ESP32PWM::adjustFrequencyLocal(double freq, float dutyScaled) {
	timerFreqSet[getTimer()] = (long) freq;
	myFreq =freq;
	if (attached()) {
		ledcDetachPin(pin);
		// Remove the PWM during frequency adjust
		_ledcSetupTimerFreq(getChannel(), freq, resolutionBits);
		writeScaled(dutyScaled);
		ledcAttachPin(pin, getChannel());  // re-attach the pin after frequency adjust
	} else {
		 _ledcSetupTimerFreq(getChannel(), freq, resolutionBits);
		writeScaled(dutyScaled);
	}
}
void ESP32PWM::adjustFrequency(double freq, float dutyScaled) {
	writeScaled(dutyScaled);
	for (int i = 0; i < timerCount[getTimer()]; i++) {
		int pwm = timerAndIndexToChannel(getTimer(), i);
		if (ChannelUsed[pwm] != NULL) {
			if (ChannelUsed[pwm]->myFreq != freq) {
				ChannelUsed[pwm]->adjustFrequencyLocal(freq,
						ChannelUsed[pwm]->getDutyScaled());
			}
		}
	}
}
double ESP32PWM::writeTone(double freq) {
	for (int i = 0; i < timerCount[getTimer()]; i++) {
			int pwm = timerAndIndexToChannel(getTimer(), i);
			if (ChannelUsed[pwm] != NULL) {
				if (ChannelUsed[pwm]->myFreq != freq) {
					ChannelUsed[pwm]->adjustFrequencyLocal(freq,
											ChannelUsed[pwm]->getDutyScaled());
				}
			}
		}

	return 0;
}
double ESP32PWM::writeNote(note_t note, uint8_t octave) {
	const uint16_t noteFrequencyBase[12] = {
	    //   C        C#       D        Eb       E        F       F#        G       G#        A       Bb        B
	        4186,    4435,    4699,    4978,    5274,    5588,    5920,    6272,    6645,    7040,    7459,    7902
	    };

	    if(octave > 8 || note >= NOTE_MAX){
	        return 0;
	    }
	double noteFreq =  (double)noteFrequencyBase[note] / (double)(1 << (8-octave));
	return writeTone( noteFreq);
}
uint32_t ESP32PWM::read() {
	return ledcRead(getChannel());
}
double ESP32PWM::readFreq() {
	return myFreq;
}
void ESP32PWM::attach(int p) {
	pin = p;
	attachedState = true;
}
void ESP32PWM::attachPin(uint8_t pin) {

	if (hasPwm(pin)) {
		attach(pin);
		ledcAttachPin(pin, getChannel());
	} else {
		Serial.println(
				"ERROR PWM channel unavailible on pin requested! " + String(pin)
						+ "\r\nPWM availible on: 2,4,5,12-19,21-23,25-27,32-33");
		return;
	}
	//Serial.print(" on pin "+String(pin));
}
void ESP32PWM::attachPin(uint8_t pin, double freq, uint8_t resolution_bits) {

	if (hasPwm(pin))
		setup(freq, resolution_bits);
	attachPin(pin);
}
void ESP32PWM::detachPin(int pin){
	ledcDetachPin(pin);
	deallocate();
}
/* Side effects of frequency changes happen because of shared timers
 *
 * LEDC Chan to Group/Channel/Timer Mapping
 ** ledc: 0  => Group: 0, Channel: 0, Timer: 0
 ** ledc: 1  => Group: 0, Channel: 1, Timer: 0
 ** ledc: 2  => Group: 0, Channel: 2, Timer: 1
 ** ledc: 3  => Group: 0, Channel: 3, Timer: 1
 ** ledc: 4  => Group: 0, Channel: 4, Timer: 2
 ** ledc: 5  => Group: 0, Channel: 5, Timer: 2
 ** ledc: 6  => Group: 0, Channel: 6, Timer: 3
 ** ledc: 7  => Group: 0, Channel: 7, Timer: 3
 ** ledc: 8  => Group: 1, Channel: 0, Timer: 0
 ** ledc: 9  => Group: 1, Channel: 1, Timer: 0
 ** ledc: 10 => Group: 1, Channel: 2, Timer: 1
 ** ledc: 11 => Group: 1, Channel: 3, Timer: 1
 ** ledc: 12 => Group: 1, Channel: 4, Timer: 2
 ** ledc: 13 => Group: 1, Channel: 5, Timer: 2
 ** ledc: 14 => Group: 1, Channel: 6, Timer: 3
 ** ledc: 15 => Group: 1, Channel: 7, Timer: 3
 */

bool ESP32PWM::checkFrequencyForSideEffects(double freq) {

	allocatenext(freq);
	for (int i = 0; i < timerCount[getTimer()]; i++) {
		int pwm = timerAndIndexToChannel(getTimer(), i);

		if (pwm == pwmChannel)
			continue;
		if (ChannelUsed[pwm] != NULL)
			if (ChannelUsed[pwm]->getTimer() == getTimer()) {
				double diff = abs(ChannelUsed[pwm]->myFreq - freq);
				if (abs(diff) > 0.1) {
					Serial.println(
							"\tWARNING PWM channel " + String(pwmChannel)
									+ " shares a timer with channel "
									+ String(pwm) + "\n"
											"\tchanging the frequency to "
									+ String(freq)
									+ " Hz will ALSO change channel "
									+ String(pwm)
									+ " \n\tfrom its previous frequency of "
									+ String(ChannelUsed[pwm]->myFreq) + " Hz\n"
											" ");
					ChannelUsed[pwm]->myFreq = freq;
				}
			}
	}
	return true;
}

ESP32PWM* pwmFactory(int pin) {
	for (int i = 0; i < NUM_PWM; i++)
		if (ESP32PWM::ChannelUsed[i] != NULL) {
			if (ESP32PWM::ChannelUsed[i]->getPin() == pin)
				return ESP32PWM::ChannelUsed[i];
		}
	return NULL;
}
