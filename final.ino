#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display setup
#define SCREEN_WIDTH 128   // Width of OLED display in pixels
#define SCREEN_HEIGHT 32   // Height of OLED display in pixels
#define OLED_RESET -1      // Reset pin (not used here)
#define SCREEN_ADDRESS 0x3C // I2C address of OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button, LED, and Buzzer pins
const int buttonPins[4] = {2, 3, 4, 5};  // Button pins
const int ledPins[4] = {8, 9, 10, 11};   // LED pins
const int buzzer = A0;  // Buzzer connected to Analog pin A0

// Game variables
const int maxLevels = 50;  // Maximum levels in the game
int ledSequence[maxLevels]; // Array to store the LED sequence
int playerSequence[maxLevels]; // Array to store player input sequence
int currentLevel = 0;   // Tracks current level
int playerIndex = 0;    // Tracks player's progress in current sequence
int score = 0;          // Player's score
bool gameStarted = false; // Flag to indicate if the game has started
bool gameLost = false;    // Flag to indicate if the player has lost

void setup() {
  Serial.begin(9600); // Initialize serial communication

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Infinite loop if display initialization fails
  }
  display.display(); // Show Adafruit splash screen
  delay(2000);
  display.clearDisplay(); // Clear the display

  // Set up button and LED pins
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); // Enable internal pull-up resistors for buttons
    pinMode(ledPins[i], OUTPUT);          // Set LED pins as output
  }
  pinMode(buzzer, OUTPUT); // Set buzzer pin as output

  // Display welcome message
  displayMessage("Welcome!", "Press button to start");
  Serial.println("Welcome to the Memory Game!");
  Serial.println("Press any button to start.");

  randomSeed(analogRead(0)); // Seed random number generator using noise from pin A0
}

void loop() {
  if (!gameStarted) { // Check if game has not started yet
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) { // Check if any button is pressed
        delay(200); // Debounce delay
        gameStarted = true;
        currentLevel = 1;
        score = 0;
        displayMessage("Game started!", "");
        Serial.println("Game started!");
        generateSequence(); // Start the game by generating a sequence
        return;
      }
    }
  } else if (gameLost) { // Check if player lost the game
    // Display game over message
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Game Over!");
    display.print("Score: ");
    display.println(score);
    display.println("Press any button to restart.");
    display.display();
    Serial.print("Game Over! Score: ");
    Serial.println(score);
    Serial.println("Press any button to restart.");
    
    playGameOverSound(); // Play game over sound
    resetGame(); // Reset game variables
  } else {
    playLevel(); // Continue playing
  }
}

// Generates a new sequence for the current level
void generateSequence() {
  ledSequence[currentLevel - 1] = random(0, 4); // Generate random LED index
  display.clearDisplay();
  displayMessage("Watch the sequence!", "");

  // Display and play sequence for the player
  for (int i = 0; i < currentLevel; i++) {
    int ledIndex = ledSequence[i];
    digitalWrite(ledPins[ledIndex], HIGH); // Turn on LED
    playBuzzerTone(ledIndex); // Play corresponding sound
    delay(500);
    digitalWrite(ledPins[ledIndex], LOW); // Turn off LED
    delay(500);
  }

  displayMessage("Your turn!", "Repeat the sequence.");
  Serial.println("Your turn! Repeat the sequence.");
  playerIndex = 0; // Reset player index
}

// Handles player input and checks for correctness
void playLevel() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) { // Check if button is pressed
      delay(200); // Debounce delay
      
      playerSequence[playerIndex] = i; // Store player input
      digitalWrite(ledPins[i], HIGH); // Turn on corresponding LED
      playBuzzerTone(i); // Play corresponding sound
      delay(100);
      digitalWrite(ledPins[i], LOW); // Turn off LED

      if (playerSequence[playerIndex] != ledSequence[playerIndex]) { // Check if input is correct
        gameLost = true; // Player lost the game
        return;
      }

      playerIndex++;
      if (playerIndex == currentLevel) { // If player completed the sequence
        score = currentLevel;
        currentLevel++;
        if (currentLevel > maxLevels) { // If max level reached
          displayMessage("You win!", "Congrats!");
          Serial.println("You win! Congrats!");
          resetGame();
        } else {
          delay(1500); // Delay before next level
          generateSequence(); // Generate next sequence
        }
        return;
      }
    }
  }
}

// Plays a sound corresponding to the pressed button
void playBuzzerTone(int ledIndex) {
  int frequencies[4] = {100, 150, 200, 250}; // Frequencies for each button
  tone(buzzer, frequencies[ledIndex], 200); // Play sound for 200ms
}

// Plays a beep-boop sound when the game is lost
void playGameOverSound() {
  tone(buzzer, 100, 300); // Beep
  delay(350);
  tone(buzzer, 50, 500);  // Boop (Lower frequency)
  delay(500);
  noTone(buzzer); // Stop buzzer
}

// Displays a message on the OLED screen
void displayMessage(const char *line1, const char *line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  display.println(line2);
  display.display();
}

// Resets the game to its initial state
void resetGame() {
  gameStarted = false;
  gameLost = false;
  currentLevel = 0;
  playerIndex = 0;
  score = 0;
}
