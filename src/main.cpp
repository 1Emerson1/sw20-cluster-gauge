#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

// Create display object (pins configured in platformio.ini)
TFT_eSPI tft = TFT_eSPI();

void testFillScreen() {
  Serial.println("Testing fill screen...");
  tft.fillScreen(TFT_BLACK);
  delay(500);
  tft.fillScreen(TFT_RED);
  delay(500);
  tft.fillScreen(TFT_GREEN);
  delay(500);
  tft.fillScreen(TFT_BLUE);
  delay(500);
  tft.fillScreen(TFT_BLACK);
}

void testText() {
  Serial.println("Testing text display...");
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.println("GC9A01");
  tft.println("Display Test");

  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1);
  tft.println();
  tft.println("1.28\" Round LCD");
  tft.println("240x240 pixels");
  tft.println();
  tft.setTextColor(TFT_CYAN);
  tft.println("TFT_eSPI Library");

  delay(2000);
}

void testShapes() {
  Serial.println("Testing shapes...");
  tft.fillScreen(TFT_BLACK);

  // Draw circles
  for (int i = 0; i < 120; i += 10) {
    tft.drawCircle(120, 120, i, TFT_BLUE);
    delay(50);
  }

  delay(1000);
  tft.fillScreen(TFT_BLACK);

  // Draw rectangles
  for (int i = 0; i < 100; i += 10) {
    tft.drawRect(120 - i, 120 - i, i * 2, i * 2, TFT_GREEN);
    delay(50);
  }

  delay(1000);
  tft.fillScreen(TFT_BLACK);

  // Draw lines
  for (int i = 0; i < 240; i += 10) {
    tft.drawLine(0, 0, i, 239, TFT_RED);
    delay(30);
  }

  delay(1000);
}

void testGauge() {
  Serial.println("Testing gauge display...");
  tft.fillScreen(TFT_BLACK);

  // Draw gauge outline
  tft.drawCircle(120, 120, 110, TFT_WHITE);
  tft.drawCircle(120, 120, 108, TFT_WHITE);

  // Draw tick marks
  for (int i = 0; i <= 180; i += 20) {
    float angle = (i - 90) * PI / 180.0;
    int x1 = 120 + cos(angle) * 100;
    int y1 = 120 + sin(angle) * 100;
    int x2 = 120 + cos(angle) * 90;
    int y2 = 120 + sin(angle) * 90;
    tft.drawLine(x1, y1, x2, y2, TFT_WHITE);
  }

  // Add numbers
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(50, 180);
  tft.print("0");
  tft.setCursor(110, 40);
  tft.print("90");
  tft.setCursor(185, 180);
  tft.print("180");

  // Draw center circle
  tft.fillCircle(120, 120, 10, TFT_RED);

  // Animate gauge needle
  for (int i = 0; i <= 180; i += 5) {
    // Erase previous needle
    if (i > 0) {
      float prevAngle = (i - 5 - 90) * PI / 180.0;
      int prevX = 120 + cos(prevAngle) * 80;
      int prevY = 120 + sin(prevAngle) * 80;
      tft.drawLine(120, 120, prevX, prevY, TFT_BLACK);
    }

    // Draw new needle
    float angle = (i - 90) * PI / 180.0;
    int x = 120 + cos(angle) * 80;
    int y = 120 + sin(angle) * 80;
    tft.drawLine(120, 120, x, y, TFT_YELLOW);
    tft.fillCircle(120, 120, 10, TFT_RED);

    delay(30);
  }

  delay(1000);
}

void testColorBars() {
  Serial.println("Testing color bars...");
  tft.fillScreen(TFT_BLACK);

  int barHeight = 240 / 7;
  tft.fillRect(0, 0, 240, barHeight, TFT_RED);
  tft.fillRect(0, barHeight, 240, barHeight, TFT_ORANGE);
  tft.fillRect(0, barHeight * 2, 240, barHeight, TFT_YELLOW);
  tft.fillRect(0, barHeight * 3, 240, barHeight, TFT_GREEN);
  tft.fillRect(0, barHeight * 4, 240, barHeight, TFT_CYAN);
  tft.fillRect(0, barHeight * 5, 240, barHeight, TFT_BLUE);
  tft.fillRect(0, barHeight * 6, 240, barHeight, TFT_MAGENTA);

  delay(2000);
}

void testPixelAccuracy() {
  Serial.println("Testing pixel accuracy...");
  tft.fillScreen(TFT_BLACK);

  // Draw a crosshair to check center alignment
  tft.drawLine(120, 0, 120, 240, TFT_WHITE);
  tft.drawLine(0, 120, 240, 120, TFT_WHITE);

  // Draw corner markers
  tft.fillCircle(0, 0, 5, TFT_RED);
  tft.fillCircle(239, 0, 5, TFT_GREEN);
  tft.fillCircle(0, 239, 5, TFT_BLUE);
  tft.fillCircle(239, 239, 5, TFT_YELLOW);

  delay(2000);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nGC9A01 Display Test");
  Serial.println("====================");
  Serial.print("Initializing display...");

  // Initialize display
  tft.init();
  tft.setRotation(0);  // 0-3 rotations available

  Serial.println("Done!");

  // Clear screen
  tft.fillScreen(TFT_BLACK);

  // Display startup message
  tft.setCursor(50, 100);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.println("READY!");

  tft.setTextSize(1);
  tft.setCursor(40, 140);
  tft.setTextColor(TFT_WHITE);
  tft.println("Starting tests...");

  delay(2000);
}

void loop() {
  // Run through all test functions
  testFillScreen();
  testText();
  testShapes();
  testColorBars();
  testPixelAccuracy();
  testGauge();

  // Wait before repeating
  Serial.println("Tests complete. Restarting in 3 seconds...\n");
  delay(3000);
}
