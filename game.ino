#include <FastLED.h>

// --- НАЛАШТУВАННЯ ЗАЛІЗА ---
#define NUM_LEDS 60        // Кількість пікселів
#define LED_PIN 6          
#define GREEN_BTN 2        
#define RED_BTN 3          
#define BLUE_BTN 4         
#define LEVEL_UP_BTN 5     

CRGB leds[NUM_LEDS];

// Кольори гри
CRGB colors[3] = {CRGB::Red, CRGB::Blue, CRGB::Green};

// --- ІГРОВІ ЗМІННІ ---
int currentLevel = 1;      
int successfulHits = 0;    

// Таймери для руху
unsigned long lastEnemyMoveTime = 0;
unsigned long lastShotMoveTime = 0;

// Змінні для кнопок (відслідковування зміни стану)
bool lastRedState = HIGH;
bool lastGreenState = HIGH;
bool lastBlueState = HIGH;
bool lastLevelUpState = HIGH;
unsigned long lastDebounceTime = 0;
const int debounceDelay = 50; // Короткий антибрязкіт

// Структури об'єктів
struct FallingPixel {
  int pos;
  CRGB color;
  bool active;
};

struct ShotPixel {
  int pos;
  CRGB color;
  bool active;
};

// Збільшений ліміт об'єктів
const int MAX_OBJECTS = 40;
FallingPixel enemies[MAX_OBJECTS];
ShotPixel shots[MAX_OBJECTS];

int spawnDistanceCounter = 0;
int nextSpawnDistance = 0;

void setup() {
  Serial.begin(9600);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  pinMode(GREEN_BTN, INPUT_PULLUP);
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(BLUE_BTN, INPUT_PULLUP);
  pinMode(LEVEL_UP_BTN, INPUT_PULLUP);
  
  resetGame();
}

void loop() {
  handleButtons();
  
  // Визначення швидкості
  int enemySpeedDelay = 1000 / (currentLevel + 1); 
  int shotSpeedDelay = enemySpeedDelay / 2; // Постріл вдвічі швидший

  bool needsDraw = false;

  // Рух пострілу
  if (millis() - lastShotMoveTime >= shotSpeedDelay) {
    moveShots();
    checkCollisions(); 
    lastShotMoveTime = millis();
    needsDraw = true;
  }

  // Рух ворогів
  if (millis() - lastEnemyMoveTime >= enemySpeedDelay) {
    moveEnemies();
    checkCollisions(); 
    spawnEnemies();
    lastEnemyMoveTime = millis();
    needsDraw = true;
  }
  
  if (needsDraw) {
    drawGame();
  }
  
  // Авто-підвищення рівня
  if (successfulHits >= 50 && currentLevel < 5) {
    levelUp();
    successfulHits = 0;
  }
}

void handleButtons() {
  bool readingRed = digitalRead(RED_BTN);
  bool readingGreen = digitalRead(GREEN_BTN);
  bool readingBlue = digitalRead(BLUE_BTN);
  bool readingLevelUp = digitalRead(LEVEL_UP_BTN);

  // Перевірка тільки при натисканні (зміна з HIGH на LOW) з урахуванням антибрязкоту
  if (millis() - lastDebounceTime > debounceDelay) {
    if (readingRed == LOW && lastRedState == HIGH) { 
      shoot(CRGB::Red); 
      lastDebounceTime = millis(); 
    }
    if (readingGreen == LOW && lastGreenState == HIGH) { 
      shoot(CRGB::Green); 
      lastDebounceTime = millis(); 
    }
    if (readingBlue == LOW && lastBlueState == HIGH) { 
      shoot(CRGB::Blue); 
      lastDebounceTime = millis(); 
    }
    if (readingLevelUp == LOW && lastLevelUpState == HIGH) { 
      if (currentLevel < 5) levelUp(); 
      lastDebounceTime = millis(); 
    }
  }

  // Зберігаємо поточний стан для наступного циклу
  lastRedState = readingRed;
  lastGreenState = readingGreen;
  lastBlueState = readingBlue;
  lastLevelUpState = readingLevelUp;
}

void shoot(CRGB color) {
  for (int i = 0; i < MAX_OBJECTS; i++) {
    if (!shots[i].active) {
      shots[i].pos = 1; // Вилітає одразу НАД базою
      shots[i].color = color;
      shots[i].active = true;
      break;
    }
  }
}

void spawnEnemies() {
  spawnDistanceCounter++;
  if (spawnDistanceCounter >= nextSpawnDistance) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
      if (!enemies[i].active) {
        enemies[i].pos = NUM_LEDS - 1; 
        enemies[i].color = colors[random(0, 3)];
        enemies[i].active = true;
        spawnDistanceCounter = 0;
        nextSpawnDistance = random(1, 6);
        break;
      }
    }
  }
}

void moveEnemies() {
  for (int i = 0; i < MAX_OBJECTS; i++) {
    if (enemies[i].active) {
      enemies[i].pos--;
      
      // Якщо ворог торкнувся бази (0-й піксель)
      if (enemies[i].pos <= 0) {
        drawGame(); // Малюємо кадр удару
        delay(300); // Даємо гравцю 300 мс, щоб побачити, хто долетів
        resetGame(); 
        return; 
      }
    }
  }
}

void moveShots() {
  for (int i = 0; i < MAX_OBJECTS; i++) {
    if (shots[i].active) {
      shots[i].pos++;
      if (shots[i].pos >= NUM_LEDS) {
        shots[i].active = false; 
      }
    }
  }
}

void checkCollisions() {
  for (int e = 0; e < MAX_OBJECTS; e++) {
    if (!enemies[e].active) continue;
    
    for (int s = 0; s < MAX_OBJECTS; s++) {
      if (!shots[s].active) continue;
      
      if (enemies[e].pos == shots[s].pos || enemies[e].pos == shots[s].pos - 1) {
        
        if (enemies[e].color == shots[s].color) {
          // Правильне влучання
          enemies[e].active = false;
          shots[s].active = false;
          successfulHits++;
        } else {
          // Неправильне влучання: ворог ГАРАНТОВАНО змінює колір на інший
          CRGB oldColor = enemies[e].color;
          CRGB newColor;
          do {
            newColor = colors[random(0, 3)];
          } while (newColor == oldColor);
          
          enemies[e].color = newColor; 
          shots[s].active = false; 
        }
      }
    }
  }
}

void levelUp() {
  currentLevel++;
  FastLED.clear();
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
  delay(100);
  FastLED.clear();
  FastLED.show();
}

void resetGame() {
  currentLevel = 1;
  successfulHits = 0;
  spawnDistanceCounter = 0;
  nextSpawnDistance = random(1, 6);
  
  for (int i = 0; i < MAX_OBJECTS; i++) {
    enemies[i].active = false;
    shots[i].active = false;
  }
  
  // Спалах програшу
  FastLED.clear();
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();
}

void drawGame() {
  FastLED.clear();
  
  // Підсвітка бази гравця (слабкий білий)
  leds[0] = CRGB(20, 20, 20); 
  
  // Малювання об'єктів
  for (int i = 0; i < MAX_OBJECTS; i++) {
    if (enemies[i].active && enemies[i].pos > 0 && enemies[i].pos < NUM_LEDS) {
      leds[enemies[i].pos] = enemies[i].color;
    }
    if (shots[i].active && shots[i].pos > 0 && shots[i].pos < NUM_LEDS) {
      leds[shots[i].pos] = shots[i].color;
    }
  }
  
  FastLED.show();
}
