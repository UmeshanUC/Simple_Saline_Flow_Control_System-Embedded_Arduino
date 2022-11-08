#include <Key.h>
#include <Keypad.h>
#include <Keypad.h>
#include <Arduino.h>
#include <TM1637Display.h>
#include <EEPROM.h>
#include "PID_v1.h"

//// Display connection pins (Digital Pins)
#define CLK 12
#define DIO 10

#define FLASH_DELAY 100
#define SENSING_DURATION 1
#define SENSOR_PIN 13
#define MORTOR_CLAMP_PIN 11
#define MORTOR_RELEASE_PIN 3


// Modes
#define DROPSIZE_SETTING_MODE 0
#define READING_MODE 1
#define RATE_SETTING_MODE 2


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
void SaveDropSize(int val);
void SaveRequiredRate(int val);
void OnChangeMode(int currentOpState);
void Print(const char* msg);
void RecoverDropSize();
void Print(const char* key, int value);
void LogParameters();
void RecoverRequiredRate();
void SetDropSize();
void OnTicking();
int CalFlowRate();
void CheckSensor();
void Control();
void InitPins();
void Actuate(int controlSignal);
// void InitController();
void MotorController(int contSig);




TM1637Display display(CLK, DIO);

int tempDisplayFlag = 0;
int timerFlag = 0;

int currentDisplayValue = NULL;  // The value to be displayed always
int opState = 1;
char customKey;
int dropSize = NULL;  // No. of drops/ml
int requiredRate = NULL;
int currentRate = 28;
int sensorCount = 0;
double Kp = 7;
////


// //PID Variables
// double refSig;  // reference
// double InputSig;
// double ControlSig;  // Control Signal (Output)
// //PID parameters
// double Kp = 0, Ki = 10, Kd = 0;

// // create PID instance
// PID pidController(&InputSig, &ControlSig, &refSig, Kp, Ki, Kd, DIRECT);

//// Keypad Variables
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 0, 2, 4, 5 };
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
// Create ---- symbol
const uint8_t segAllDash[] = {
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
  SEG_G   // -
};
// Create M1 symbol
const uint8_t segM1[] = {
  SEG_A | SEG_B | SEG_E | SEG_F,  // M_1
  SEG_F | SEG_A | SEG_B | SEG_C,  // M_2
  SEG_B | SEG_C                   //1
};



void setup() {
  Serial.begin(9600);
  InitPins();

  // Timer Interrupt
  cli();

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;  //initialize counter value to 0

  // set compare match registers
  OCR1A = 15624;  // = (16*10^6) / (1*1024) - 1 (must be <65536)

  // CTC mode
  TCCR1B |= (1 << WGM12);

  // 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);

  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);


  sei();  //allow interrupts

  /////////
  // InitController();
  display.setBrightness(1);
  AllFlashDisplay();
  RecoverDropSize();
  RecoverRequiredRate();

  LogParameters();
}




// Main Loop
void loop() {
  customKey = CheckKeypress();

  if (timerFlag) {
    OnTicking();
    timerFlag = 0;
  }

  CheckOpState();
  CheckSensor();

  Actuate(requiredRate);


  delay(50);
}

//ISR

ISR(TIMER1_COMPA_vect) {
  //timer1 interrupt 1Hz
  timerFlag = 1;
}

////


void AppendToDisplay(uint8_t digit) {
  if (tempDisplayFlag) {
    currentDisplayValue = NULL;
    tempDisplayFlag = 0;
  }

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
    ChangeMode();
  } else if (customKey == 'A') {
    SetDropSize();
  }
  return customKey;
}

// On Confirming Input
void ChangeMode() {
  int prevOpState = opState;
  if (opState < 2) {
    opState++;
  } else {
    opState = 1;
  }
  OnChangeMode(prevOpState);
  // ClearDisplay();
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
  delay(250);
}

bool IsNumericChar(char character) {
  if (character >= 48 && character <= 58) {
    return true;
  }
  return false;
}


void CheckOpState() {

  // check for recovered drop size
  if (dropSize <= 0) {
    opState = 0;
  }

  switch (opState) {
    case DROPSIZE_SETTING_MODE:
    case RATE_SETTING_MODE:
      if (customKey) {
        if (IsNumericChar(customKey)) {
          AppendToDisplay(CharToAsciiInt(customKey));
        }
      } else if (currentDisplayValue != NULL) {
        display.showNumberDec(currentDisplayValue);
      }
      break;

    case READING_MODE:
      display.showNumberDec(currentRate);
      break;
  }
}

// val <= 255
void SaveDropSize(int size) {
  EEPROM.put(0, size);
  dropSize = size;
}


void SaveRequiredRate(int rate) {
  EEPROM.put(4, rate);
  requiredRate = rate;
}

// Callback on changing mode
void OnChangeMode(int prevOpState) {
  switch (prevOpState) {
    case DROPSIZE_SETTING_MODE:
      SaveDropSize(currentDisplayValue);
      break;

    case RATE_SETTING_MODE:
      SaveRequiredRate(currentDisplayValue);
      break;

    case READING_MODE:
      break;
  }

  switch (opState) {
    case DROPSIZE_SETTING_MODE:
      display.showNumberDec(dropSize);
      break;

    case RATE_SETTING_MODE:
      if (requiredRate) {
        display.showNumberDec(requiredRate);
      } else {
        display.setSegments(segAllDash);
      }
      tempDisplayFlag = 1;
      break;

    case READING_MODE:
      break;
  }
  LogParameters();
}

void Print(const char* msg) {
  Serial.println(msg);
}

void Print(const char* key, int value) {
  Serial.print(key);
  Serial.println(value);
}

void RecoverDropSize() {
  EEPROM.get(0, dropSize);
  Print("Drop Size loaded: ", dropSize);
}

void LogParameters() {
  Print("Operating State: ", opState);
  Print("Required Rate: ", requiredRate);
  Print("Current Rate: ", currentRate);
  Print("Drop Size: ", dropSize);
}

void RecoverRequiredRate() {
  EEPROM.get(4, requiredRate);
  Print("Required Rate loaded: ", requiredRate);
}

void SetDropSize() {
  if (opState == 0) {
    ChangeMode();
    return;
  }
  opState = 0;
  currentDisplayValue = dropSize;
  tempDisplayFlag = 1;
  Print("Operating Mode: ", opState);
}

void OnTicking() {
  currentRate = CalFlowRate();
  Serial.println(sensorCount);
  sensorCount = 0;  // Reset counter for sensor inputs
}

int CalFlowRate() {
  int qty = sensorCount * dropSize;   // Flown amount in ml
  int rate = qty / SENSING_DURATION;  // flow rate

  return (rate);
}

void CheckSensor() {
  if (digitalRead(SENSOR_PIN) == HIGH) {
    sensorCount++;
    // Serial.println(1);
  } else {
    // Serial.println(0);
  }
}



void InitPins() {
  pinMode(MORTOR_CLAMP_PIN, OUTPUT);
  pinMode(MORTOR_RELEASE_PIN, OUTPUT);
}


void Actuate(int reqRate) {
  int err = reqRate - currentRate;
  while (abs(err * Kp) > 255) {
    if (Kp < 0.01) break;
    Kp = Kp - 0.01;
  }
  int contSig = err * Kp;
  Print("Control Sig: ", contSig);
  MotorController(contSig);
}

// void InitController() {
//   pidController.SetMode(AUTOMATIC);
//   pidController.SetTunings(Kp, Ki, Kd);
// }

void Control() {
  // InputSig = currentRate;
  // refSig = requiredRate;

  // pidController.Compute();
}

void MotorController(int contSig) {
  if (contSig > 0) {
    analogWrite(MORTOR_CLAMP_PIN, 255);
    analogWrite(MORTOR_RELEASE_PIN, 0);
  } else if (contSig < 0) {
    analogWrite(MORTOR_RELEASE_PIN, 255);
    analogWrite(MORTOR_CLAMP_PIN, 0);
  }
}