/*
 * analogWrite.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: Harry-Laptop
 */



#include "analogWrite.h"

static  uint8_t CESP32PWMTimers[ 2 ] = {0};
static  uint8_t CESP32PWMPinMap[ 40 ] = {0};

  void analogWrite( uint8_t APin, uint16_t AValue )
  {
//    Serial.println( "analogWrite" );
//    Serial.println( AValue );
    if( AValue == 0 || AValue == 255 )
    {
      if(  CESP32PWMPinMap[ APin ] != 0 )
      {
        ledcWrite(  CESP32PWMPinMap[ APin ] - 1, AValue );
//        Serial.println( "ledcDetachPin" );
        digitalWrite( APin, (AValue == 255 ) ? HIGH : LOW );
        ledcDetachPin( APin );
        uint8_t AChannel =  CESP32PWMPinMap[ APin ] - 1;
         CESP32PWMTimers[ AChannel >> 3 ] &= ~ ( 1 << ( AChannel & 7 ) );
         CESP32PWMPinMap[ APin ] = 0;
      }

      digitalWrite( APin, (AValue == 255 ) ? HIGH : LOW );
      return;
    }

    if(  CESP32PWMPinMap[ APin ] == 0 )
    {
      uint8_t AChannel = 0;
      for( AChannel = 0; AChannel < 16; ++AChannel )
        if( ! (  CESP32PWMTimers[ AChannel >> 3 ] & ( 1 << ( AChannel & 7 ) ) ))
          break;

      if( AChannel == 16 )
        return; // Can't allocate channel

       CESP32PWMPinMap[ APin ] = AChannel + 1;
      // CESP32PWMTimers[ AChannel >> 3 ] != ( 1 << ( AChannel & 7 ) );
      ledcSetup( AChannel, 1000, 8 ); // 1KHz 8 bit
      ledcWrite( AChannel, AValue );
      ledcAttachPin( APin, AChannel );
//      Serial.print( "ledcAttachPin: " ); Serial.print( APin ); Serial.print( " " ); Serial.println( AChannel );
      return;
    }

    ledcWrite(  CESP32PWMPinMap[ APin ] - 1, AValue );
//    Serial.print( "ledcWrite: " ); Serial.print(  CESP32PWMPinMap[ APin ] - 1 ); Serial.print( " " ); Serial.println( AValue );
  }
