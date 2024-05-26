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

int piezo = 3;

int btnLcurrent = LOW;
int btnRcurrent = LOW;
int btnLprevious = LOW;
int btnRprevious = LOW;

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
int screen[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int O[2] = { 0b11000000, 0b11000000 };
int L[4] = { 0b10000000, 0b10000000, 0b11000000, 0b00000000 };
int J[4] = { 0b00000001, 0b00000001, 0b00000011, 0b00000000 };
int T[3] = { 0b00010000, 0b00111000, 0b00000000 };
int I[4] = { 0b10000000, 0b10000000, 0b10000000, 0b10000000 };
int Z[3] = { 0b11000000, 0b01100000, 0b00000000 };
int S[3] = { 0b01100000, 0b11000000, 0b00000000 };

int blockType = 0;

bool active = false;

int score = 0; 

int lostCount =0;

void loadNewFigure() {
  blockType = random(7);
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
      lcd.print("You lost ");
      lcd.print(lostCount);
      lcd.print(" times");
      lcd.setCursor(0,1);
      lcd.print("Scann card again");
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scann card to");
      lcd.setCursor(0,1);
      lcd.print("play");
    }
  }
  else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Your score : ");
    lcd.setCursor(0,1);
    lcd.print(score);
    
    timeNow = millis();

    // Read button states
    btnLcurrent = digitalRead(btnL);
    btnRcurrent = digitalRead(btnR);

    // Handle left button press
    if (btnLcurrent == HIGH && btnLprevious == LOW) {
      moveLeftTime = timeNow;
      moveLeft();
      tone(piezo,350,200);
    } else if (btnLcurrent == HIGH && timeNow - moveLeftTime >= moveDelayTime) {
      moveLeftTime = timeNow;
      moveLeft();
      tone(piezo,350,200);
    }

    // Handle right button press
    if (btnRcurrent == HIGH && btnRprevious == LOW) {
      moveRightTime = timeNow;
      moveRight();
      tone(piezo,400,200);
    } else if (btnRcurrent == HIGH && timeNow - moveRightTime >= moveDelayTime) {
      moveRightTime = timeNow;
      moveRight();
      tone(piezo,400,200);
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