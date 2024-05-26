#include <LedControl.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

int melody[] = {
  NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5
};

int noteDurations[] = {
  200, 200, 200, 200, 200, 200, 200, 200
};

LiquidCrystal_I2C lcd(0x27,16,2); 

// Pin definitions for matrix
#define DATA_IN 7
#define CLK 5
#define CS 6
#define MAX_DEVICES 2

LedControl lc = LedControl(DATA_IN, CLK, CS, MAX_DEVICES);

#define RST_PIN 9    
#define SS_PIN 10    
MFRC522 mfrc522(SS_PIN, RST_PIN);

int btnL = 2;
int btnR = 4;
int btnRotate = 8;

int piezo = 3;

int btnLcurrent = LOW;
int btnRcurrent = LOW;
int btnLprevious = LOW;
int btnRprevious = LOW;
int btnRotatecurrent = LOW;
int btnRotateprevious = LOW;

// Piece position and type
int x = 0; // x position of the piece (column)
int y = 0; // y position of the piece (row)

// Timing variables
int timeNow = 0;
int previousTime = 0;
int moveLeftTime = 0;
int moveRightTime = 0;
int delayTime = 500;
int moveDelayTime = 250; 

int figure[8];
int currentFigure;
int angle = 0;
int screen[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int O[4][2] = {
  { 0b11000000, 0b11000000 },
  { 0b11000000, 0b11000000 },
  { 0b11000000, 0b11000000 },
  { 0b11000000, 0b11000000 }
};

int L[4][4] = {
  { 0b10000000, 0b10000000, 0b11000000, 0b00000000 },
  { 0b11100000, 0b10000000, 0b00000000, 0b00000000 },
  { 0b11000000, 0b01000000, 0b01000000, 0b00000000 },
  { 0b00000000, 0b00100000, 0b11100000, 0b00000000 }
};

int J[4][4] = {
  { 0b00000001, 0b00000001, 0b00000011, 0b00000000 },
  { 0b00000000, 0b00000100, 0b00000111, 0b00000000 },
  { 0b00000011, 0b00000010, 0b00000010, 0b00000000 },
  { 0b00000000, 0b00000111, 0b00000001, 0b00000000 }
};

int T[4][3] = {
  { 0b00010000, 0b00111000, 0b00000000 },
  { 0b00010000, 0b00011000, 0b00010000 },
  { 0b00000000, 0b00111000, 0b00010000 },
  { 0b00010000, 0b00110000, 0b00010000 }
};

int I[4][4] = {
  { 0b10000000, 0b10000000, 0b10000000, 0b10000000 },
  { 0b00000000, 0b11110000, 0b00000000, 0b00000000 },
  { 0b00100000, 0b00100000, 0b00100000, 0b00100000 },
  { 0b00000000, 0b00000000, 0b11110000, 0b00000000 }
};

int Z[4][3] = {
  { 0b11000000, 0b01100000, 0b00000000 },
  { 0b00100000, 0b01100000, 0b01000000 },
  { 0b00000000, 0b11000000, 0b01100000 },
  { 0b10000000, 0b11000000, 0b01000000 }
};

int S[4][3] = {
  { 0b01100000, 0b11000000, 0b00000000 },
  { 0b01000000, 0b01100000, 0b00100000 },
  { 0b00000000, 0b01100000, 0b11000000 },
  { 0b10000000, 0b11000000, 0b01000000 }
};


int blockType = 0;

bool active = false;

int score = 0;

int lostCount = 0;

void loadNewFigure() {
  blockType = random(7);
  currentFigure = blockType;
  angle = 0;
  getFigure(blockType);
  x = 0;
  y = -figureHeight(); // Start the figure above the visible area

  moveLeftTime = millis();// Reset the move timers when a new figure is loaded
  moveRightTime = millis();
}

void setup() {
  for (int i = 0; i < MAX_DEVICES; i++) { // Initialize the matrix
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
 
  pinMode(btnL, INPUT);
  pinMode(btnR, INPUT);
  pinMode(btnRotate, INPUT);

  loadNewFigure();// Load the first figure
}

void loop() {
  if (!active) { // if not active
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      active = true;
      resetGame();
    }

    if(lostCount > 0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("You Scored: ");
      lcd.setCursor(0,1);
      lcd.print(score);
      delay(1000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("You Lost ");
      lcd.print(lostCount);
      if(lostCount == 1)
        lcd.print(" Time");
      else
        lcd.print(" Times");
      lcd.setCursor(0,1);
      lcd.print("Scan Card Again");
      delay(1000 );
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan Card to");
      lcd.setCursor(0,1);
      lcd.print("Play");
    }
  }
  else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Your Score:");
    lcd.setCursor(0,1);
    lcd.print(score);
    
    timeNow = millis();

    btnLcurrent = digitalRead(btnL);
    btnRcurrent = digitalRead(btnR);
    btnRotatecurrent = digitalRead(btnRotate);

    if (btnLcurrent != btnLprevious && btnLcurrent == HIGH) {
      moveLeftTime = timeNow;
      moveLeft();
      tone(piezo,350,200);
    } else if (btnLcurrent == HIGH && timeNow - moveLeftTime >= moveDelayTime) {
      moveLeftTime = timeNow;
      moveLeft();
      tone(piezo,350,200);
    }

    if (btnRcurrent != btnRprevious && btnRcurrent == HIGH) {
      moveRightTime = timeNow;
      moveRight();
      tone(piezo,400,200);
    } else if (btnRcurrent == HIGH && timeNow - moveRightTime >= moveDelayTime) {
      moveRightTime = timeNow;
      moveRight();
      tone(piezo,400,200);
    }
    if(btnRotatecurrent != btnRotateprevious && btnRotatecurrent == HIGH){
      rotateFigure(currentFigure, angle);
      angle++;
      tone(piezo,450,200);
    }

    btnLprevious = btnLcurrent;
    btnRprevious = btnRcurrent;
    btnRotateprevious = btnRotatecurrent;

    if (timeNow - previousTime >= delayTime) { // piece drop with delay time
      previousTime = timeNow;
      dropFigure();
    }

    updateScreen();

    if (checkLose()) {
      Serial.println("You lost!");
      active = false;
    }
  }
}
void rotateFigure(int num, int angle) {
  int oldLeft = x;
  int oldRight = x + figureWidth() - 1;

  int newAngle = (angle + 1) % 4;

  if (canRotate(newAngle)) {
    getFigureRotated(num, newAngle);

    int newLeft = x;
    int newRight = x + figureWidth() - 1;

    // Calculate the necessary shifts to align the shape properly after rotation
    int shiftLeft = oldLeft - newLeft;
    int shiftRight = oldRight - newRight;

    // Shift the shape to its correct position
    if (shiftLeft > 0) {
      for (int i = 0; i < shiftLeft; i++) {
        moveLeft();
      }
    } else if (shiftRight < 0) {
      for (int i = 0; i < -shiftRight; i++) {
        moveRight();
      }
    }
  }
}

bool canRotate(int newAngle) {
  int tempFigure[8];
  memcpy(tempFigure, figure, sizeof(figure));
  int tempAngle = angle;
  angle = newAngle;
  getFigure(currentFigure);
  angle = tempAngle;

  // Calculate the necessary shifts to align the shape properly after rotation
  int oldLeft = x;
  int oldRight = x + figureWidth() - 1;
  int newLeft = x;
  int newRight = x + figureWidth() - 1;
  int shiftLeft = oldLeft - newLeft;
  int shiftRight = oldRight - newRight;

  // Shift the shape to its correct position
  if (shiftLeft > 0) {
    for (int i = 0; i < shiftLeft; i++) {
      moveLeft();
    }
  } else if (shiftRight < 0) {
    for (int i = 0; i < -shiftRight; i++) {
      moveRight();
    }
  }

  // Check for collisions after rotation and shifting
  for (int i = 0; i < figureHeight(); i++) {
    if ((figure[i] & screen[y + i]) != 0) {
      return false; // Collision detected, cannot rotate
    }
  }
  return true;
}


void getFigure(int num) {
  memset(figure, 0, sizeof(figure));
  switch (num) {
    case 0: memcpy(figure, O[angle], sizeof(O[angle])); break;
    case 1: memcpy(figure, L[angle], sizeof(L[angle])); break;
    case 2: memcpy(figure, J[angle], sizeof(J[angle])); break;
    case 3: memcpy(figure, T[angle], sizeof(T[angle])); break;
    case 4: memcpy(figure, I[angle], sizeof(I[angle])); break;
    case 5: memcpy(figure, Z[angle], sizeof(Z[angle])); break;
    case 6: memcpy(figure, S[angle], sizeof(S[angle])); break;
  }
}
void getFigureRotated(int num, int angle) {
  switch (num) {
    case 0: memcpy(figure, O[angle], sizeof(O[angle])); break;
    case 1: memcpy(figure, L[angle], sizeof(L[angle])); break;
    case 2: memcpy(figure, J[angle], sizeof(J[angle])); break;
    case 3: memcpy(figure, T[angle], sizeof(T[angle])); break;
    case 4: memcpy(figure, I[angle], sizeof(I[angle])); break;
    case 5: memcpy(figure, Z[angle], sizeof(Z[angle])); break;
    case 6: memcpy(figure, S[angle], sizeof(S[angle])); break;
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
    score++;
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
      score++;
      return true;
    }
  }
  return false;
}

bool checkLose() {
  if (screen[0] != 0) {  // Check the top row for any blocks
    playLosingMelody();
    lostCount++;
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
  for (int i = 0; i < MAX_DEVICES; i++) { // Clear the display
    lc.clearDisplay(i);
  }
  
  for (int i = 0; i < 16; i++) { // Display the screen 
    if (i < 8) {
      lc.setRow(0, i, screen[i]);
    } else {
      lc.setRow(1, i - 8, screen[i]);
    }
  }

  for (int i = 0; i < figureHeight(); i++) {  // Display the current figure
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
      score += 10;
      for (int j = i; j > 0; j--) { // Shift all rows above down
        screen[j] = screen[j - 1];
      }
      screen[0] = 0;
      i--; // Check the same row again in case of consecutive full lines
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
int figureWidth() {
  int maxWidth = 0;
  for (int i = 0; i < figureHeight(); i++) {
    int width = 0;
    for (int j = 0; j < 8; j++) {
      if (figure[i] & (1 << j)) {
        width = j + 1;
      }
    }
    if (width > maxWidth) {
      maxWidth = width;
    }
  }
  return maxWidth;
}


void resetGame(){
  memset(screen, 0, sizeof(screen));
  score = 0;
  loadNewFigure();
  previousTime = millis();
  moveLeftTime = millis();
  moveRightTime = millis();
}

void playLosingMelody() {
  for (int i = 0; i < 8; i++) {
    tone(piezo, melody[i], noteDurations[i]);
    delay(noteDurations[i] * 1.3);
  }
  noTone(piezo); 
}
