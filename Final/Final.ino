#include <Keypad.h>
#include <Arduino.h>
#include <TM1637Display.h>

//// Display connection pins (Digital Pins)
#define CLK 11
#define DIO 10

#define FLASH_DELAY 100

// Modes
#define READING_MODE 0
#define SETTING_MODE 1

#define USE_TIMER_1 true  // To use timer1 interrupt

//// Prototypes
void AppendToDisplay(uint8_t digit);
int CharToAsciiInt(char val);
char CheckKeypress();
void ChangeMode();
void ZeroFlashDisplay();
void ZeroSlowFlashDisplay();
void AllFlashDisplay();
void CheckOpState();
void ClearDisplay();
bool IsNumericChar(char character);


TM1637Display display(CLK, DIO);

int confirmFlag = 0;
int currentDisplayValue = 0;  // The value to be displayed always
int opState = 0;
char customKey;

//// Keypad Variables
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 2, 3, 4, 5 };
byte colPins[COLS] = { 6, 7, 8, 9 };

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


//// Display Valriables

// Create an array that turns all segments ON
const uint8_t allON[] = { 0xff, 0xff, 0xff, 0xff };
// Create an array that turns all segments OFF
const uint8_t allOFF[] = { 0x00, 0x00, 0x00, 0x00 };
// Create 0000 symbol
const uint8_t segAllZero[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F   // O
};



void setup() {
  Serial.begin(9600);

  display.setBrightness(1);
  AllFlashDisplay();
}




// Main Loop
void loop() {
  customKey = CheckKeypress();

  CheckOpState();




  // Finally Display the value
  if (currentDisplayValue != NULL) {
    display.showNumberDec(currentDisplayValue);
  }
}



void AppendToDisplay(uint8_t digit) {
  if (currentDisplayValue == 0) {
    currentDisplayValue = digit;
  } else {
    String currentDisplayValStr = String(currentDisplayValue);
    if (currentDisplayValStr.length() >= 4) {
      return;
    }

    String digitStr = String(digit);
    currentDisplayValStr.concat(digitStr);
    currentDisplayValue = currentDisplayValStr.toInt();
    Serial.println(currentDisplayValue);
  }
}

int CharToAsciiInt(char val) {
  if (val >= 48 && val <= 57) {
    Serial.println(int(val - 48));
    return int(val - 48);
  }
  return 0;
}

char CheckKeypress() {
  char customKey = customKeypad.getKey();
  if (customKey == 'D') {
    confirmFlag = 1;
    ChangeMode();
  }
  return customKey;
}

// On Confirming Input
void ChangeMode() {
  if (opState < 1) {
    opState++;
  } else {
    opState = 0;
  }
  // ClearDisplay();
  Serial.println(opState);
}

void AllFlashDisplay() {
  display.setSegments(allON);
  delay(FLASH_DELAY);
  display.setSegments(allOFF);
  delay(FLASH_DELAY);
}

void ZeroFlashDisplay() {
  display.setSegments(segAllZero);
  delay(FLASH_DELAY);
  display.setSegments(allOFF);
  delay(FLASH_DELAY);
}

void ZeroSlowFlashDisplay() {
  display.setSegments(segAllZero);
  delay(FLASH_DELAY * 2);
  display.setSegments(allOFF);
  delay(FLASH_DELAY * 2);
}


void ClearDisplay() {
  currentDisplayValue = NULL;
}

bool IsNumericChar(char character) {
  if (character >= 48 && character <= 58) {
    return true;
  }
  return false;
}


void CheckOpState() {
  switch (opState) {
    case SETTING_MODE:
      ZeroSlowFlashDisplay();
      if (customKey) {
        Serial.println(customKey);
        if (IsNumericChar(customKey)) {
          AppendToDisplay(CharToAsciiInt(customKey));
        }
      }
      break;

    case READING_MODE:
      AllFlashDisplay();
      break;
  }
}