#include <LedControl.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2); 

// Pin definitions for MAX7219
#define DATA_IN 7
#define CLK 5
#define CS 6
#define MAX_DEVICES 2 // Adjusted to 2 as you are using two 8x8 matrices

LedControl lc = LedControl(DATA_IN, CLK, CS, MAX_DEVICES);

#define RST_PIN 9    
#define SS_PIN 10    
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Button pin definitions
int btnL = 3;
int btnR = 2;

// Button state variables
int btnLcurrent = LOW;
int btnRcurrent = LOW;
int btnLprevious = LOW;
int btnRprevious = LOW;

// Piece position and type
int x = 0; // x position of the piece (column)
int y = 0; // y position of the piece (row)
int blockType = 0;

// Timing variables
int timeNow = 0;
int previousTime = 0;
int moveLeftTime = 0;
int moveRightTime = 0;
int delayTime = 500;
int moveDelayTime = 250; // Delay for continuous movement

// Screen and figure representation
int figure[8];
int screen[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Blocks definition
int O[2] = { 0b11000000, 0b11000000 };
int L[4] = { 0b10000000, 0b10000000, 0b11000000, 0b00000000 };
int J[4] = { 0b00000001, 0b00000001, 0b00000011, 0b00000000 };
int T[3] = { 0b00010000, 0b00111000, 0b00000000 };
int I[4] = { 0b10000000, 0b10000000, 0b10000000, 0b10000000 };
int Z[3] = { 0b11000000, 0b01100000, 0b00000000 };
int S[3] = { 0b01100000, 0b11000000, 0b00000000 };

bool active = false;

void loadNewFigure() {
  blockType = random(7);
  getFigure(blockType);
  x = 0;
  y = -figureHeight(); // Start the figure above the visible area

  // Reset the move timers when a new figure is loaded
  moveLeftTime = millis();
  moveRightTime = millis();
}

void setup() {
  
  // Initialize the MAX7219 with the number of devices
  for (int i = 0; i < MAX_DEVICES; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 8);
    lc.clearDisplay(i);
  }

  SPI.begin();          
  mfrc522.PCD_Init(); 

  randomSeed(analogRead(A0));
  
  lcd.init();      
  lcd.backlight();
  
  Serial.begin(9600);
  Serial.print("start");
  // Initialize button pins
  pinMode(btnL, INPUT);
  pinMode(btnR, INPUT);

  // Load the first figure
  loadNewFigure();
}

void loop() {
  if (!active) { // if not active
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      active = true;
      resetGame();
      Serial.println("Card detected and activated!");
    }
    lcd.setCursor(0,0);
    lcd.print("Scann card to");
    lcd.setCursor(0,1);
    lcd.print("play");
  }
  else{
    timeNow = millis();

    // Read button states
    btnLcurrent = digitalRead(btnL);
    btnRcurrent = digitalRead(btnR);

    // Handle left button press
    if (btnLcurrent == HIGH && btnLprevious == LOW) {
      moveLeftTime = timeNow;
      moveLeft();
    } else if (btnLcurrent == HIGH && timeNow - moveLeftTime >= moveDelayTime) {
      moveLeftTime = timeNow;
      moveLeft();
    }

    // Handle right button press
    if (btnRcurrent == HIGH && btnRprevious == LOW) {
      moveRightTime = timeNow;
      moveRight();
    } else if (btnRcurrent == HIGH && timeNow - moveRightTime >= moveDelayTime) {
      moveRightTime = timeNow;
      moveRight();
    }

    btnLprevious = btnLcurrent;
    btnRprevious = btnRcurrent;

    // Handle piece drop based on delay time
    if (timeNow - previousTime >= delayTime) {
      previousTime = timeNow;
      dropFigure();
    }

    // Update the screen with the current state
    updateScreen();

    if (checkLose()) {
      Serial.println("You lost!");
      active = false;
    }
  }
}

void getFigure(int num) {
  memset(figure, 0, sizeof(figure));
  switch (num) {
    case 0: memcpy(figure, O, sizeof(O)); break;
    case 1: memcpy(figure, L, sizeof(L)); break;
    case 2: memcpy(figure, J, sizeof(J)); break;
    case 3: memcpy(figure, T, sizeof(T)); break;
    case 4: memcpy(figure, I, sizeof(I)); break;
    case 5: memcpy(figure, Z, sizeof(Z)); break;
    case 6: memcpy(figure, S, sizeof(S)); break;
  }
}

void moveLeft() {
  if (canMoveLeft()) {
    for (int i = 0; i < figureHeight(); i++) {
      figure[i] <<= 1;
    }
    x--;
  }
}

void moveRight() {
  if (canMoveRight()) {
    for (int i = 0; i < figureHeight(); i++) {
      figure[i] >>= 1;
    }
    x++;
  }
}

bool canMoveLeft() {
  for (int i = 0; i < figureHeight(); i++) {
    if ((figure[i] & 0b10000000) != 0 || (figure[i] << 1 & screen[y + i]) != 0) {
      return false;
    }
  }
  return true;
}

bool canMoveRight() {
  for (int i = 0; i < figureHeight(); i++) {
    if ((figure[i] & 0b00000001) != 0 || (figure[i] >> 1 & screen[y + i]) != 0) {
      return false;
    }
  }
  return true;
}

void dropFigure() {
  if (y + figureHeight() >= 16 || checkCollision(y + 1)) {
    mergeFigureToScreen();
    loadNewFigure();
  } else {
    y++;
  }
}

bool checkCollision(int newY) {
  if (newY < 0) return false; // No collision if the figure is above the screen
  for (int i = 0; i < figureHeight(); i++) {
    if (newY + i < 16 && (figure[i] & screen[newY + i]) != 0) {
      return true;
    }
  }
  return false;
}

bool checkLose() {
  // Check the top row for any blocks
  if (screen[0] != 0) {
    return true;
  }
  return false;
}

void mergeFigureToScreen() {
  for (int i = 0; i < figureHeight(); i++) {
    if (y + i >= 0 && y + i < 16) {
      screen[y + i] |= figure[i];
    }
  }
  checkLine();
}

void updateScreen() {
  // Clear the display and redraw
  for (int i = 0; i < MAX_DEVICES; i++) {
    lc.clearDisplay(i);
  }
  
  // Display the screen state
  for (int i = 0; i < 16; i++) {
    if (i < 8) {
      lc.setRow(0, i, screen[i]);
    } else {
      lc.setRow(1, i - 8, screen[i]);
    }
  }

  // Display the current figure
  for (int i = 0; i < figureHeight(); i++) {
    if (y + i >= 0 && y + i < 16) {
      if (y + i < 8) {
        lc.setRow(0, y + i, screen[y + i] | figure[i]);
      } else {
        lc.setRow(1, y + i - 8, screen[y + i] | figure[i]);
      }
    }
  }
}

void checkLine() {
  for (int i = 0; i < 16; i++) {
    if (screen[i] == 0b11111111) {
      // Shift all rows above down
      for (int j = i; j > 0; j--) {
        screen[j] = screen[j - 1];
      }
      // Clear the top row
      screen[0] = 0;
      // Check the same row again in case of consecutive full lines
      i--;
    }
  }
}

int figureHeight() {
  for (int i = 7; i >= 0; i--) {
    if (figure[i] != 0) {
      return i + 1;
    }
  }
  return 0;
}

void resetGame(){
  lc.clearDisplay(0);
  lc.clearDisplay(1);

   figure[8];
   screen[16];
}
