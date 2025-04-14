#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Adjust I2C address if needed

// LED pins
const int greenLedPin = 5;
const int yellowLedPin = 6;
const int redLedPin = 7;

// Button pins
const int buttonAPin = 4;
const int buttonBPin = 3;
const int buttonCPin = 2;
const int buttonDPin = 9;

// Security settings
const char disarmCode[] = "AAAA";
const int disarmCodeLength = 4;
int disarmCodeIndex = 0;
int wrongAttempts = 0;
const int maxWrongAttempts = 4;

// Timer variables
unsigned long previousMillis = 0;
const long interval = 1000;
int seconds = 15;
bool alarmActive = true;
bool systemDisarmed = false;

// LED flashing
unsigned long redLedFlashMillis = 0;
const long redFlashInterval = 250;  // Faster flashing

// Button handling
unsigned long lastButtonPressTime = 0;
const long debounceTime = 50;
bool buttonPressed = false;

void resetSystem() {
  alarmActive = true;
  systemDisarmed = false;
  seconds = 15;
  disarmCodeIndex = 0;
  wrongAttempts = 0;
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
  lcd.clear();
  lcd.print("TIME: 15");
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.print("TIME: 15");

  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buttonAPin, INPUT_PULLUP);
  pinMode(buttonBPin, INPUT_PULLUP);
  pinMode(buttonCPin, INPUT_PULLUP);
  pinMode(buttonDPin, INPUT_PULLUP);
}

void loop() {
  if (systemDisarmed) {
    return;  // Stop if disarmed
  }

  checkButtons();
  updateTimer();
  updateDisplay();
}

void checkButtons() {
  // Check all buttons with proper debouncing
  if (digitalRead(buttonAPin) == LOW && !buttonPressed) {
    handleButtonPress('A');
  } 
  else if (digitalRead(buttonBPin) == LOW && !buttonPressed) {
    handleButtonPress('B');
  }
  else if (digitalRead(buttonCPin) == LOW && !buttonPressed) {
    handleButtonPress('C');
  }
  else if (digitalRead(buttonDPin) == LOW && !buttonPressed) {
    handleButtonPress('D');
  }
  else if (digitalRead(buttonAPin) == HIGH && 
           digitalRead(buttonBPin) == HIGH &&
           digitalRead(buttonCPin) == HIGH &&
           digitalRead(buttonDPin) == HIGH) {
    buttonPressed = false;  // Reset when all buttons released
  }
}

void handleButtonPress(char button) {
  if (millis() - lastButtonPressTime < debounceTime) {
    return;  // Debounce check
  }
  
  buttonPressed = true;
  lastButtonPressTime = millis();

  if (button == disarmCode[disarmCodeIndex]) {
    disarmCodeIndex++;
    wrongAttempts = 0;  // Reset wrong attempts on correct press
    
    if (disarmCodeIndex == disarmCodeLength) {
      systemDisarmed = true;
      alarmActive = false;
      digitalWrite(greenLedPin, HIGH);
      lcd.clear();
      lcd.print("Access GRANTED");
    }
  } 
  else {
    disarmCodeIndex = 0;  // Reset code progress
    wrongAttempts++;     // Count wrong attempts
    
    if (wrongAttempts >= maxWrongAttempts) {
      digitalWrite(yellowLedPin, HIGH);
      delay(1000);
      digitalWrite(yellowLedPin, LOW);
      wrongAttempts = 0;  // Reset after showing warning
    }
  }
}

void updateTimer() {
  if (!alarmActive) return;

  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    seconds--;

    // Flash red LED faster when below 5 seconds
    if (seconds < 5) {
      if (currentMillis - redLedFlashMillis >= redFlashInterval) {
        redLedFlashMillis = currentMillis;
        digitalWrite(redLedPin, !digitalRead(redLedPin));
      }
    } 
    else {
      digitalWrite(redLedPin, LOW);
    }

    if (seconds < 0) {
      alarmActive = false;
      digitalWrite(redLedPin, HIGH);
      lcd.clear();
      lcd.print("Access DENIED");
      delay(2000);
      resetSystem();
    }
  }
}

void updateDisplay() {
  if (alarmActive) {
    lcd.setCursor(6, 0);
    lcd.print("  ");  // Clear old number
    lcd.setCursor(6, 0);
    lcd.print(seconds);
  }
}