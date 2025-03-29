const int ledPin = 2;  // D2 (GPIO 2) for blinking LED
const int inputPins[] = {32, 33, 34, 35, 36, 39};  // Input pins
const int numInputs = sizeof(inputPins) / sizeof(inputPins[0]);
const int numMotors = 20;
const int motorIDS[numMotors] = {13, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 0, 2, 4, 5, 12, 14, 15};

void setup() {
  pinMode(ledPin, OUTPUT);  

  // Set input pins
  for (int i = 0; i < numInputs; i++) {
    pinMode(inputPins[i], INPUT);  // GPIO 34-39 do not support pull-ups
  }

  for (int i = 0; i < numMotors; i++) {
    pinMode(motorIDS[i], OUTPUT);  // Set each pin as an outputa
  }
}

void loop() {
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
}
