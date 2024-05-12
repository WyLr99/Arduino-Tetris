#include <LedControl.h>

const int piezo = 3;

const int btnLeft = ;
const int btnRight = ;

const btnLeftCurrent = LOW;
const btnRightCurrent = LOW;

const btnLeftPrivious = LOW;
const btnRightPrivious = LOW;

// Pin definitions for MAX7219
#define DATA_IN 12
#define CLK 11
#define CS 10
// Number of MAX7219 devices connected
#define MAX_DEVICES 1

// Create an instance of the LedControl class
LedControl lc = LedControl(DATA_IN, CLK, CS, MAX_DEVICES);

bool screen[8][8];

void setup() {
  pinMode(btnLeft, INPUT);
  pinMode(btnRight, INPUT);
  pinMode(piezo, OUTPUT);

  // Initialize the MAX7219 with the number of devices
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);      
  lc.clearDisplay(0); 

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      screen[row][col] = false; 
    }
  }
  
  randomSeed(A0);
  Serial.begin(9600);
}

void loop() {
  //movement
   btnLeftCurrent = digitalRead(btnLeft);
   btnRightCurrent = digitalRead(btnRight);

   if(btnLeftCurrent != btnLeftPrivious){
    if(btnLeftCurrent == HIGH){
      //left
    }
   }

    if(btnRightCurrent != btnRightPrivious){
      if(btnRightCurrent == HIGH){
        //right
      }
    }

   for(int i = 0; i < 8; i++) {
    if(i > 0) {
        screen[i-1][1] = false;
        lc.setLed(0, i-1, 1, false);
    }
    screen[i][1] = true;
    lc.setLed(0, i, 1, true);

    if(checkBelow(i, 1)){
      break;
      };

    delay(500);
   }

   btnLeftPrivious = btnLeftCurrent;
   btnRightPrivious = btnRightCurrent;
}

bool checkBelow(int y,int x){
  if(y == 8)return true;
  return screen[y+1][x];
}
