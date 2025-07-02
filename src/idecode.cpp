#include <Bluepad32.h>
#include "driver/pcnt.h"

#define PCNT_UNIT_FROM_ID(ID) PCNT_UNIT_##ID

#define BUTTON_PRESS_TO_MODULE_ID(x) \
    ((x) == 0x04 ? 0 : \
    (x) == 0x08 ? 1 : \
    (x) == 0x02 ? 2 : \
    -1)

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Motor DIR and EN pins (DIR1 / EN1 and DIR2 / EN2)
const int motorDirPins[6] = {25, 32, 5, 16, 15, 0};   // DIR1 or DIR2
const int motorEnPins[6]  = {26, 33, 18, 17, 2, 4};   // EN1 or EN2

// Encoder A and B phase pins (Pin 5 & 6 from each connector J1–J6)
const int encoderAPins[6] = {39, 35, 19, 22, 14, 12}; // Encoder A
const int encoderBPins[6] = {36, 34, 21, 23, 27, 13}; // Encoder B

//------------------------------------------Motor class------------------------------------------------

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
    if(encoderInvertDirection == 1){
      pcntConfig.pos_mode         = PCNT_COUNT_DEC;
      pcntConfig.neg_mode         = PCNT_COUNT_INC;     
    }else if(encoderInvertDirection == -1){
      pcntConfig.pos_mode         = PCNT_COUNT_INC;
      pcntConfig.neg_mode         = PCNT_COUNT_DEC;     
    }

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

  int16_t getEncoderCount() const{
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

  void setEncoderInvertDirection(int new_encoderInvertDirection){
    encoderInvertDirection = new_encoderInvertDirection;
  }


  private:
    pcnt_unit_t pcntUnit;
    gpio_num_t gpioA;
    gpio_num_t gpioB;
    float PIDconstants[3] = {};
    int invertDirection;
    int encoderInvertDirection;
    int prevEncoderCount = 0;
    long previousTime = 0;
    float speedPrevious = 0;
    float ePPrevious = 0;
    float eI;
    float previousPower;

};

//-----------------------------------------Module Class------------------------------------------


class Module {
  public: 

    Module(Motor &new_motor1, Motor &new_motor2):
        motor1(new_motor1),
        motor2(new_motor2)
    {}

    void setupEncoders(){
      motor1.setupEncoder();
      motor2.setupEncoder();
    }

    void configInversions(int motor1Invert, int motor2Invert, int motor1EncoderInvert, int motor2EncoderInvert){
        motor1.setInvertDirection(motor1Invert);
        motor2.setInvertDirection(motor2Invert);
        motor1.setEncoderInvertDirection(motor1EncoderInvert);
        motor2.setEncoderInvertDirection(motor2EncoderInvert);
    }

    void driveRaw(int power, int direction){
        motor1.driveRaw(power, direction);
        motor2.driveRaw(power, -1 * direction);
    }
    
    void turn(int power, int direction){
        motor1.driveRaw(power, direction);
        motor2.driveRaw(power, direction);
    }

    void stop(){
      motor1.stop();
      motor2.stop();
    }

    void turnToAngle(float targetAngle) {
        long currentTime = micros();
        float dT = ((float)(currentTime - previousTime)) / 1.0e6;

        float eP = targetAngle - getAngle();
        eI = eI + (eP * dT);
        float eD = (eP - ePPrevious) / dT;

        int power = (int)((eP * PIDconstants[0]) + (eI * PIDconstants[1]) + (eD * PIDconstants[2]));

        if (abs(power) > 255) {
        if (power < 0) {
            power = -255;
        } else {
            power = 255;
        }
        }

        // Serial.print(power);
        // Serial.print(" | ");

        turn(power, 1);
    }

    void displayEncoderReadings(){
      Serial.print(motor1.getEncoderCount());
      Serial.print(" | ");
      Serial.print(motor2.getEncoderCount());
      Serial.print(" | ");
      Serial.println(motor1.getEncoderCount() - motor2.getEncoderCount());
    }

    void update() {
        

    }

    void setTargetPower(float new_targetPower) {
        targetPower = new_targetPower;
    }

    void setTargetAngle(float new_targetAngle) {
        targetAngle = new_targetAngle;
    }

    float getTargetAngle(){
        return targetAngle;
    }

    void resetAngle(){
      currentAngle = 0;
    }

    float getAngle(){
      return currentAngle;
    }
    
    void updateAngle(){
      currentAngle += (motor1.getEncoderCount() - motor2.getEncoderCount()) * 360 / 7550;
      while(currentAngle > 360){
        currentAngle -= 360;
      }
      while(currentAngle < 0){
        currentAngle += 360;
      }
      motor1.resetEncoder();
      motor2.resetEncoder();
    }

  private:
    Motor& motor1;
    Motor& motor2;
    float currentAngle;
    float PIDconstants[6] = {};
    float previousTime = 0;
    float eI = 0;
    float ePPrevious = 0;
    float targetAngle = 0;
    float targetPower = 0;
    int prevState = 0;  //0 --> turning, 1 --> speed control
};

Motor motor[6] = {
  Motor(0, motorDirPins[0], motorEnPins[0], PCNT_UNIT_FROM_ID(0), encoderAPins[0], encoderBPins[0]),
  Motor(1, motorDirPins[1], motorEnPins[1], PCNT_UNIT_FROM_ID(1), encoderAPins[1], encoderBPins[1]),
  Motor(2, motorDirPins[2], motorEnPins[2], PCNT_UNIT_FROM_ID(2), encoderAPins[2], encoderBPins[2]),
  Motor(3, motorDirPins[3], motorEnPins[3], PCNT_UNIT_FROM_ID(3), encoderAPins[3], encoderBPins[3]),
  Motor(4, motorDirPins[4], motorEnPins[4], PCNT_UNIT_FROM_ID(4), encoderAPins[4], encoderBPins[4]),
  Motor(5, motorDirPins[5], motorEnPins[5], PCNT_UNIT_FROM_ID(5), encoderAPins[5], encoderBPins[5])
};

Module module[3] = {
  Module(motor[0], motor[1]),
  Module(motor[2], motor[3]),
  Module(motor[4], motor[5])
};

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons(),  // bitmask of pressed "misc" buttons
        ctl->gyroX(),        // Gyro X
        ctl->gyroY(),        // Gyro Y
        ctl->gyroZ(),        // Gyro Z
        ctl->accelX(),       // Accelerometer X
        ctl->accelY(),       // Accelerometer Y
        ctl->accelZ()        // Accelerometer Z
    );
}

void processGamepad(ControllerPtr ctl) {
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...
    if(ctl->buttons() == 0x08 || ctl->buttons() == 0x04 || ctl->buttons() == 0x02){
      if(ctl->axisX() > 500){
        int moduleID = BUTTON_PRESS_TO_MODULE_ID(ctl->buttons());
        // Serial.print("TURNING M");
        // Serial.print(moduleID);
        // Serial.println(" CW");
        module[moduleID].turn(255, 1);
        module[moduleID].updateAngle();
        Serial.println(module[moduleID].getAngle());
      }else if(ctl->axisX() < -500){
        int moduleID = BUTTON_PRESS_TO_MODULE_ID(ctl->buttons());
        // Serial.print("TURNING M");
        // Serial.print(moduleID);
        // Serial.println(" CCW");
        module[moduleID].turn(255, -1);
        module[moduleID].updateAngle();
        Serial.println(module[moduleID].getAngle());
      }else if(ctl->axisY() > 500){
        int moduleID = BUTTON_PRESS_TO_MODULE_ID(ctl->buttons());
        // Serial.print("TURNING M");
        // Serial.print(moduleID);
        // Serial.println(" CCW");
        module[moduleID].driveRaw(255, 1);
        module[moduleID].updateAngle();
        Serial.println(module[moduleID].getAngle());
      }else if(ctl->axisY() < -500){
        int moduleID = BUTTON_PRESS_TO_MODULE_ID(ctl->buttons());
        // Serial.print("TURNING M");
        // Serial.print(moduleID);
        // Serial.println(" CCW");
        module[moduleID].driveRaw(255, -1);
        module[moduleID].updateAngle();
        Serial.println(module[moduleID].getAngle());
      }else{
        for(int i = 0; i < 3; i++){
          module[i].stop();
        }
      }
    }else if(ctl->buttons() == 0x01){
      module[0].resetAngle();
      module[1].resetAngle();
      module[2].resetAngle();
      Serial.println("Reset angles!");
    }else if(ctl->axisY() > 500){
      for(int i = 0; i < 3; i++){
          module[i].driveRaw(255, 1);
      }
    }else if(ctl->axisY() < -500){
      for(int i = 0; i < 3; i++){
          module[i].driveRaw(255, -1);
      }
    }else{
      for(int i = 0; i < 3; i++){
        module[i].stop();
      }
    }

    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.
    // dumpGamepad(ctl);
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    module[0].configInversions(1, -1, -1, 1);
    module[1].configInversions(1, -1, -1, 1);
    module[2].configInversions(1, 1, -1, 1);

    for(int i = 0; i < 3; i++){
      module[i].setupEncoders();
    }

    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);
}

// Arduino loop function. Runs in CPU 1.
void loop() {
    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();
    // module1.turn(255, 1);

    delay(50);
}
