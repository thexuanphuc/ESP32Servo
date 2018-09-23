#include <ESP32Servo.h>
int APin=13;
ESP32PWM  pwm;
int freq = 1000;
void setup() {
  Serial.begin(115200);
  pwm.attachPin(APin,freq, 10);// 1KHz 10 bit

}
void loop() {

    // fade the LED on thisPin from off to brightest:
    for (float brightness = 0; brightness <=1; brightness+=0.001) {
      pwm.writeScaled(brightness);
      delay(2);
    }
    delay(1000);
    // fade the LED on thisPin from brithstest to off:
    for (float brightness = 1; brightness >= 0; brightness-=0.001) {
      freq+=10;
      pwm.writeTone(freq);// update the time base of the PWM
      pwm.writeScaled(brightness);
      delay(2);
    }
    // pause between LEDs:
    delay(1000);
    freq=1000;
    pwm.writeTone(freq);// reset the fequency values
}
