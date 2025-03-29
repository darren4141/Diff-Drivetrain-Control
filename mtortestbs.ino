const int numMotors = 7;
// const int motorIDS[numMotors] = {13, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
const int motorIDS[numMotors] = {0, 2, 4, 5, 12, 14, 15};


void setup() {
  Serial.begin(115200);  // Start serial monitor
  for (int i = 0; i < numMotors; i++) {
    pinMode(motorIDS[i], OUTPUT);  // Set each pin as an output
  }
}

void loop() {
  for (int i = 0; i < numMotors; i++) {
    digitalWrite(motorIDS[i], HIGH); // Turn motor ON
    delay(500);  // Wait 500ms
    digitalWrite(motorIDS[i], LOW); // Turn motor OFF
    delay(500);  // Wait 500ms
  }
}
