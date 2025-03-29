const int ledPin = 2;  // D2 (GPIO 2) for blinking LED
const int inputPins[] = {34, 35, 36, 39, 25, 26, 22, 23, 21, 19, 16, 4};  // Input pins
const int numInputs = sizeof(inputPins) / sizeof(inputPins[0]);
const int numMotors = 20;
const int motorIDS[numMotors] = {13, 12, 14, 27, 33, 32, 15, 2, 0, 17, 5, 18};

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
  bool inputDetected = false;

  // Check if any input pin is HIGH
  for (int i = 0; i < numInputs; i++) {
    if (digitalRead(inputPins[i]) == HIGH) {
      inputDetected = true;
      break;  // No need to check further
    }
  }

  // Blink LED if input is detected
  if (inputDetected) {
    digitalWrite(ledPin, HIGH);
  }else{
    digitalWrite(ledPin, LOW);
  }
}
