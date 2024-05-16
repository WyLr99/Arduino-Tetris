#include <LedControl.h>

// Pin definitions for MAX7219
#define DATA_IN 12
#define CLK 11
#define CS 10
#define MAX_DEVICES 2
LedControl lc = LedControl(DATA_IN, CLK, CS, MAX_DEVICES);
int figure[8];
int screen[8] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
//Blocks
int O[8] =
{
  0b11000000,
  0b11000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
 
int L[8] =
{
  0b10000000,
  0b10000000,
  0b11000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
 
int J[8] =
{
  0b00000001,
  0b00000001,
  0b00000011,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
 
int T[8] =
{
  0b00010000,
  0b00111000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
 
int I[8] =
{
  0b10000000,
  0b10000000,
  0b10000000,
  0b10000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
 
int Z[8] =
{
  0b11000000,
  0b01100000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
 
int S[8] =
{
  0b01100000,
  0b11000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};

  void checkCollision() {
    // Check if the block has collided with another block
    for (int i = 0; i < 8; i++) {
      if ((figure[i] & screen[i]) != 0) {
        // Collision detected
        // Handle collision here
        break;
      }
    }

    // Check if the block has touched the ground
    if (figure[7] != 0) {
      // Block has touched the ground
      // Handle ground touch here
    }
  }

 
void setup() {
  // Initialize the MAX7219 with the number of devices
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);      
  lc.clearDisplay(0); 
  randomSeed(A0);
  Serial.begin(9600);
}

void loop() {
 for(int i = 0; i < 8 ; i++){
  lc.setRow(0,i,screen[i]);
 }
  int blockType = random(0, 7);

switch(blockType) {
    case 0:
        for(int i = 0; i < 8; i++) {
            figure[i] = O[i];
        }
        break;
    case 1:
        for(int i = 0; i < 8; i++) {
            figure[i] = L[i];
        }
        break;
    case 2:
        for(int i = 0; i < 8; i++) {
            figure[i] = J[i];
        }
        break;
    case 3:
        for(int i = 0; i < 8; i++) {
            figure[i] = T[i];
        }
        break;
    case 4:
        for(int i = 0; i < 8; i++) {
            figure[i] = I[i];
        }
        break;
    case 5:
        for(int i = 0; i < 8; i++) {
            figure[i] = Z[i];
        }
        break;
    case 6:
        for(int i = 0; i < 8; i++) {
            figure[i] = S[i];
        }
        break;
    default:
        // Handle invalid blockType values here
        break;
}


}