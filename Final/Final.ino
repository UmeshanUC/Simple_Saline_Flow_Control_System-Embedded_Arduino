#include <Keypad.h>
#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 11
#define DIO 12

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   2000

TM1637Display display(CLK, DIO);


const byte ROWS = 4; 
const byte COLS = 3; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {0, 1, 2, 3}; 
byte colPins[COLS] = {5, 6, 7}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
// int keyNum = 0;
uint8_t data[] = {0xff,0xff,0xff,0xff};
uint8_t blank[] = {0x00,0x00,0x00,0x00};


void setup(){
  Serial.begin(115200);
  
  display.setBrightness(0x0f);

  pinMode(8,INPUT);
  
}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  //display.setSegments(data);
  uint8_t keyNum = (uint8_t)customKey;
  
  
  // Keypad
  if (customKey){
    Serial.println(customKey);
    data[0] = display.encodeDigit(keyNum);
  }
    // TM1637
    data[1] = display.encodeDigit(0);
    data[2] = display.encodeDigit(0);
    data[3] = display.encodeDigit(0);    
    display.setSegments(data);

  if(digitalRead(8)==LOW)
  {
    Serial.print(0);
  }  
  else
  {
    Serial.print(1);
  }
  
}