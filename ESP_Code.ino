const int buttonPin = 0;  // GPIO0
const unsigned long timeout = 5000;  // 5 seconds

String morseBuffer = "";  // Morse buffer
unsigned long lastPressTime = 0;  // Last button press timestamp
bool buttonPressed = false;
bool readyToSend = false;  // <-- NEW: Flag to know when to send

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  static unsigned long pressStartTime = 0;
  
  if (digitalRead(buttonPin) == LOW) {
    if (!buttonPressed) {
      pressStartTime = millis();
      buttonPressed = true;
      Serial.println("Button Pressed!");
    }
  } else {
    if (buttonPressed) {
      unsigned long pressDuration = millis() - pressStartTime;
      buttonPressed = false;
      
      if (pressDuration < 200) {
        morseBuffer += ".";
      } else {
        morseBuffer += "-";
      }
      
      lastPressTime = millis();  // Update time of last press
      readyToSend = true;        // Reset the send flag because we got new input

      //Serial.print("Morse so far: ");
      //Serial.println(morseBuffer);
    }
  }

  // Check if 5 seconds passed after last button press
  if (readyToSend && millis() - lastPressTime >= timeout && morseBuffer.length() > 0) {
    // Now send the accumulated Morse code
    Serial.println(morseBuffer);

    // Clear everything
    morseBuffer = "";
    readyToSend = false;  // Reset sending flag
  }
}
