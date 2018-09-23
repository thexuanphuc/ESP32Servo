#include <ESP32Servo.h>
int APin=13;
ESP32PWM * pwm;
void setup() {
  Serial.begin(115200);
  pwm=new ESP32PWM();
  pwm->setup( 1000, 8); // 1KHz 8 bit
  pwm->attachPin(APin);

}
void loop() {

    // fade the LED on thisPin from off to brightest:
    for (int brightness = 0; brightness < 255; brightness++) {
      pwm->write(brightness);
      delay(2);
    }
    // fade the LED on thisPin from brithstest to off:
    for (int brightness = 255; brightness >= 0; brightness--) {
      pwm->write(brightness);
      delay(2);
    }
    // pause between LEDs:
    delay(100);
    //Serial.println(" PWM on "+String(thisPin));
  
}

