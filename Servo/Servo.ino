#include <Keypad.h>

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

void setup(){
  Serial.begin(115200);
  
  /*pinMode(13, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(13,HIGH);*/
}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    Serial.println(customKey);
  }
  
  
  /*if (customKey=='A'){
   digitalWrite(11, HIGH);
   digitalWrite(10, LOW);
   delay(400); // Wait for 1000 millisecond(s)
  }else if (customKey=='B'){
   digitalWrite(11, LOW);
   digitalWrite(10, HIGH);
   delay(400); // Wait for 1000 millisecond(s)
  }*/
}