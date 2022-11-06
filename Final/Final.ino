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
    
    data[1] = display.encodeDigit(0);
    data[2] = display.encodeDigit(0);
    data[3] = display.encodeDigit(0);    
    display.setSegments(data);

  // TM1637
  /*if(customKey)
  {
    if(customKey=='*' || customKey== '#')
    {
      keyNum = 0;
    }
    else
    {
      if(keyNum <= 999)
      {
        keyNum = (keyNum) + (int(customKey)-48);
      }
    }
  }*/

  //display.showNumberDec(keyNum);
  /*display.setBrightness(0x0f);

  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  display.setSegments(data);
  display.showNumberDec(23, false, 2,1);
  delay(TEST_DELAY);
  
  display.setSegments(data);
  display.showNumberDec(153, false, 3, 1);
  delay(TEST_DELAY);

  display.setSegments(data);
  for(int i=0; i<=500; i++)
  {
    display.showNumberDec(i);
  }*/
}