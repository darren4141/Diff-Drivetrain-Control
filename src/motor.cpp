#include <Arduino.h>
#include "driver/pcnt.h"

// PWM configuration
const int pwmFreq = 20000;       // 20 kHz
const int pwmResolution = 8;     // 8-bit resolution (0-255)
const int pwmDuty = 255;         // ~70% duty cycle

class Motor{
  public:
    int motorID;
    int dirPin;
    int enablePin;
    volatile int encoderCount;

  Motor(int new_motorID, int new_dirPin, int new_enablePin, pcnt_unit_t unit, int encPinA, int encPinB):
    motorID(new_motorID), 
    dirPin(new_dirPin), 
    enablePin(new_enablePin),
    pcntUnit(unit),
    gpioA(static_cast<gpio_num_t>(encPinA)),
    gpioB(static_cast<gpio_num_t>(encPinB))
    {
      encoderCount = 0;
      prevEncoderCount = 0;
      invertDirection = 1;

      pinMode(dirPin, OUTPUT);

      //motorID is used for PWM channel
      ledcSetup(motorID, pwmFreq, pwmResolution);
      ledcAttachPin(enablePin, motorID);        
    }
    
  void setupEncoder(){
    pcnt_config_t pcntConfig = {};

    //link gpio to pcnt, pulse counts pin edge, control determines direction
    pcntConfig.pulse_gpio_num   = gpioA;
    pcntConfig.ctrl_gpio_num    = gpioB;
    pcntConfig.channel          = PCNT_CHANNEL_0;
    pcntConfig.unit             = pcntUnit;

    //increment in rising edge, decrement on falling edge
    pcntConfig.pos_mode         = PCNT_COUNT_INC;
    pcntConfig.neg_mode         = PCNT_COUNT_DEC;

    //if control signal is low, reverse counting direction
    pcntConfig.lctrl_mode       = PCNT_MODE_REVERSE;
    //if control signal is high, keep direction the same
    pcntConfig.hctrl_mode       = PCNT_MODE_KEEP;

    //upper and lower limits for counter, max value of uint16
    pcntConfig.counter_h_lim    = 32767;
    pcntConfig.counter_l_lim    = -32768;

    pcnt_unit_config(&pcntConfig);
    
    //reject very short pulses <100 APB cycles
    pcnt_set_filter_value(pcntUnit, 100);
    pcnt_filter_enable(pcntUnit);

    pcnt_counter_pause(pcntUnit);
    pcnt_counter_clear(pcntUnit);
    pcnt_counter_resume(pcntUnit);
  }

  int16_t readEncoder() const{
    //hardware only supports uint16
    int16_t count = 0;
    pcnt_get_counter_value(pcntUnit, &count);
    return count;
  }

  void resetEncoder(){
    pcnt_counter_clear(pcntUnit);
  }

  void driveRaw(int speed, int direction){
    direction *= invertDirection;
    if(direction == 1){
      digitalWrite(dirPin, HIGH);
    }else if(direction == -1){
      digitalWrite(dirPin, LOW);
    }

    ledcWrite(motorID, speed);
  }

  void stop(){
    ledcWrite(motorID, 0);
  }

  void setInvertDirection(int new_invertDirection){
    invertDirection = new_invertDirection;
  }


  private:
    pcnt_unit_t pcntUnit;
    gpio_num_t gpioA;
    gpio_num_t gpioB;
    float PIDconstants[3] = {};
    int invertDirection;
    int prevEncoderCount = 0;
    long previousTime = 0;
    float speedPrevious = 0;
    float ePPrevious = 0;
    float eI;
    float previousPower;

};