#include <Wire.h> 
#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h> 
#include <SevSeg.h> 

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1 
#define POT_PIN A0 
#define SCORE_DISPLAY_PIN 9 
#define LIFE_LED_PIN_1 2 
#define LIFE_LED_PIN_2 3 
#define LIFE_LED_PIN_3 4 
#define START_BUTTON_PIN 5 
#define EXIT_BUTTON_PIN 6 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 
SevSeg sevseg; 

int ballX, ballY, ballSpeedX, ballSpeedY; 
int paddlePos; 
int score; 
int lives; 
int bricks[3][3]; 
int currentLevel = 1; 
bool gameStarted; 

enum GameState { 
  MAIN_MENU, 
  GAME_SCREEN, 
  GAME_OVER 
}; 

GameState gameState; 

void setup() { 
  Serial.begin(9600); 
  pinMode(POT_PIN, INPUT); 
  pinMode(SCORE_DISPLAY_PIN, OUTPUT); 
  pinMode(LIFE_LED_PIN_1, OUTPUT); 
  pinMode(LIFE_LED_PIN_2, OUTPUT); 
  pinMode(LIFE_LED_PIN_3, OUTPUT); 
  pinMode(START_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(EXIT_BUTTON_PIN, INPUT_PULLUP); 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay(); 
  display.display(); 
  gameState = MAIN_MENU; 
  resetGame(); 
  
  // Initialize SevSeg library
  byte numDigits = 2; 
  byte digitPins[] = {7, 8}; // A- pin 7, B - pin 8
  byte segmentPins[] = {9, 10, 11, 12, 13, 14, 15}; // C - pin 9, D - pin 10, E - pin 11, F - pin 12, G - pin 13
  bool resistorsOnSegments = true; 
  bool updateWithDelays = false; 
  bool leadingZeros = false; 
  bool disableDecPoint = true; 
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint); 
} 

void loop() { 
  switch (gameState) { 
    case MAIN_MENU: 
      mainMenu(); 
      break; 
    case GAME_SCREEN: 
      gameScreen(); 
      break; 
    case GAME_OVER: 
      gameOver(); 
      break; 
  } 
} 

void mainMenu() { 
  display.clearDisplay(); 
  display.setCursor(0, 0); 
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE); 
  display.println("\n"); 
  display.println(" 1. Start\n"); 
  display.println(" 2. Exit\n"); 
  display.display(); 
  if (digitalRead(START_BUTTON_PIN) == LOW) { 
    gameState = GAME_SCREEN; 
  } else if (digitalRead(EXIT_BUTTON_PIN) == LOW) { 
    display.clearDisplay(); 
    display.setCursor(0, 0); 
    display.setTextSize(1); 
    display.setTextColor(SSD1306_WHITE); 
    display.println("\nThank you for playing!\n"); 
    display.display(); 
    delay(2000); 
    gameStarted = false; 
    gameState = MAIN_MENU; 
  } 
} 

void gameScreen() { 
  if (!gameStarted) { 
    resetGame(); 
    gameStarted = true; 
  } 
  display.clearDisplay(); 
  drawBricks(); 
  drawPaddle(); 
  drawBall(); 
  drawScore(); 
  drawLives(); 
  display.display(); 
  movePaddle(); 
  moveBall(); 
  checkCollisions(); 
} 

void drawBricks() { 
  for (int i = 0; i < 3; i++) { 
    for (int j = 0; j < 3; j++) { 
      if (bricks[i][j]) { 
        int brickX = j * (40 + 2); 
        int brickY = i * (10 + 2); 
        display.fillRect(brickX, brickY, 40, 10, SSD1306_WHITE); 
      } 
    } 
  } 
} 

void drawPaddle() { 
  paddlePos = analogRead(POT_PIN) / (1023 / (128 - 40)); 
  display.fillRect(paddlePos, 58, 40, 6, SSD1306_WHITE); 
} 

void drawBall() { 
  display.fillCircle(ballX, ballY, 5, SSD1306_WHITE); 
} 

void drawScore() { 
  sevseg.setNumber(score); 
  sevseg.refreshDisplay(); 
} 

void drawLives() { 
  if (lives >= 3) { 
    digitalWrite(LIFE_LED_PIN_1, HIGH); 
    digitalWrite(LIFE_LED_PIN_2, HIGH); 
    digitalWrite(LIFE_LED_PIN_3, HIGH); 
  } else if (lives == 2) { 
    digitalWrite(LIFE_LED_PIN_1, LOW); 
    digitalWrite(LIFE_LED_PIN_2, HIGH); 
    digitalWrite(LIFE_LED_PIN_3, HIGH); 
  } else if (lives == 1) { 
    digitalWrite(LIFE_LED_PIN_1, LOW); 
    digitalWrite(LIFE_LED_PIN_2, LOW); 
    digitalWrite(LIFE_LED_PIN_3, HIGH); 
  } else { 
    digitalWrite(LIFE_LED_PIN_1, LOW); 
    digitalWrite(LIFE_LED_PIN_2, LOW); 
    digitalWrite(LIFE_LED_PIN_3, LOW); 
  } 
} 

void movePaddle() { 
  paddlePos = analogRead(POT_PIN) / (1023 / (128 - 40)); 
} 

void moveBall() { 
  ballX += ballSpeedX; 
  ballY += ballSpeedY; 
} 

void checkCollisions() { 
  if (ballY - 5 <= 0) { 
    ballSpeedY = -ballSpeedY; 
  } 
  if (ballX - 5 <= 0 || ballX + 5 >= SCREEN_WIDTH) { 
    ballSpeedX = -ballSpeedX; 
  } 
  if (ballY + 5 >= SCREEN_HEIGHT - 6 && ballX >= paddlePos && ballX <= paddlePos + 40) { 
    ballSpeedY = -ballSpeedY; 
  } 
  int brickColumn = ballX / 42; 
  int brickRow = ballY / 12; 
  if (brickRow >= 0 && brickRow < 3 && brickColumn >= 0 && brickColumn < 3 && bricks[brickRow][brickColumn]) { 
    bricks[brickRow][brickColumn] = false; 
    ballSpeedY = -ballSpeedY; 
    score++; 
    if (score == 9) { 
      currentLevel++; 
      displayLevel(); 
      resetGame(); 
    } 
  } 
  if (ballY + 5 >= SCREEN_HEIGHT) { 
    lives--; 
    if (lives <= 0) { 
      gameState = GAME_OVER; 
    } else { 
      resetBall(); 
    } 
  } 
} 

void resetGame() { 
  score = 0; 
  lives = 3; 
  ballX = 64; 
  ballY = 32; 
  ballSpeedX = random(1, 3) * (random(0, 2) == 0 ? -1 : 1); 
  ballSpeedY = random(1, 3) * (random(0, 2) == 0 ? -1 : 1); 
  for (int i = 0; i < 3; i++) { 
    for (int j = 0; j < 3; j++) { 
      bricks[i][j] = true; 
    } 
  } 
} 

void resetBall() { 
  ballX = 64; 
  ballY = 32; 
  ballSpeedX = random(1, 3) * (random(0, 2) == 0 ? -1 : 1); 
  ballSpeedY = random(1, 3) * (random(0, 2) == 0 ? -1 : 1); 
} 

void gameOver() { 
  display.clearDisplay(); 
  display.setCursor(0, 0); 
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE); 
  if (lives <= 0) { 
    display.println("Game Over"); 
    display.print("\nBricks Destroyed: "); 
    display.println(score); 
    display.println("\nYou lose!\n"); 
  } else if (currentLevel == 6) { 
    display.println("Congratulations, You Win!\n"); 
    display.print("Total Bricks Destroyed: \n"); 
    display.println(score); 
  } 
  display.display(); 
  delay(5000); 
  gameStarted = false; 
  gameState = MAIN_MENU; 
} 

void displayLevel() { 
  display.clearDisplay(); 
  display.setCursor(0, 0); 
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE); 
  display.print(" Level: "); 
  display.println(currentLevel); 
  display.display(); 
  delay(2000); 
}