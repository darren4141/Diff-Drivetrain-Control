#include "motor.cpp"

class Module {
  public: 

    Module(Motor &new_motor1, Motor &new_motor2):
        motor1(new_motor1),
        motor2(new_motor2)
    {
        motor1.setupEncoder();
        motor2.setupEncoder();
    }

    void configInversions(int motor1Invert, int motor2Invert){
        motor1.setInvertDirection(motor1Invert);
        motor2.setInvertDirection(motor2Invert);
    }

    void driveRaw(int power){
        motor1.driveRaw(power, 1);
        motor2.driveRaw(power, -1);
    }
    
    void turn(int power){
        motor1.driveRaw(power, 1);
        motor2.driveRaw(power, 1);
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

        turn(power);
    }

    int getEncoderOffset() {
        return (motor2.getEncoderCount()) + (motor1.getEncoderCount());
    }

    float getAngle() {
        return (float)(((motor1.getEncoderCount() + motor2.getEncoderCount()) % 25000) * 0.0144);  // 0.0144 is equivalent to * 360 / 25000
    }

    void update() {
        float currentAngle = getAngle();

        if (abs(currentAngle - targetAngle) > 2) {  //2 degree tolerance
        if (prevState == 1) {
            previousTime = micros();
            eI = 0;
            ePPrevious = 0;
            prevState = 0;
        }
        turnToAngle(targetAngle);
        } else {
            if (prevState == 0) {
                previousTime = micros();
                eI = 0;
                ePPrevious = 0;
                prevState = 1;
            }
            driveRaw(targetSpeed);  //temporarily 6 but i need to make a lookup table
        }

        Serial.print(motor1.getSpeed());
        Serial.print(" | ");
        Serial.print(motor2.getSpeed());
        Serial.print(" | ");
        Serial.println(abs(currentAngle - targetAngle));
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

  private:
    Motor& motor1;
    Motor& motor2;
    float PIDconstants[6] = {};
    float previousTime = 0;
    float eI = 0;
    float ePPrevious = 0;
    float targetAngle = 0;
    float targetPower = 0;
    int prevState = 0;  //0 --> turning, 1 --> speed control



}