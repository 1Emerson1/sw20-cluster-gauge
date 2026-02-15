#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

// Create display object (pins configured in platformio.ini)
TFT_eSPI tft = TFT_eSPI();

// Oil pressure sensor configuration
#define OIL_PRESSURE_PIN 34  // GPIO34 - ADC1_CH6 (input only pin, good for ADC)

// Voltage divider resistors (to scale 4.5V to 3.3V max)
// R1 = 3.9kΩ, R2 = 10kΩ
// Vout = Vin * R2 / (R1 + R2) = 4.5V * 10k / 13.9k = 3.24V
#define VOLTAGE_DIVIDER_R1 3900.0  // Ohms
#define VOLTAGE_DIVIDER_R2 10000.0 // Ohms

// Sensor voltage range (Lowdoller 7990100 specifications)
#define SENSOR_MIN_VOLTAGE 0.5  // 0 PSI
#define SENSOR_MAX_VOLTAGE 4.5  // 100 PSI
#define SENSOR_MAX_PSI 100.0

// 2GR-FE oil pressure specifications (from Toyota service manual)
// Minimum at idle (hot): 11.6 psi
// At 6000 rpm: 55.5 psi
// Normal operating range: 15-60 psi
#define OIL_PRESSURE_MIN_SAFE 10.0   // Below this = critical
#define OIL_PRESSURE_MIN_WARN 15.0   // Below this = warning
#define OIL_PRESSURE_MAX_NORM 70.0   // Above this = high (cold/high rpm)

// Display configuration
#define SCREEN_CENTER_X 120
#define SCREEN_CENTER_Y 120
#define GAUGE_RADIUS 110
#define GAUGE_START_ANGLE 135  // degrees (bottom left)
#define GAUGE_END_ANGLE 405    // degrees (bottom right, 270 degree sweep)

// 1990s Toyota MR2 color scheme
#define GAUGE_FACE_COLOR 0x0000      // Black
#define GAUGE_TEXT_COLOR 0xFFFF      // White
#define GAUGE_NEEDLE_COLOR 0xFD20    // Orange (Toyota style)
#define GAUGE_WARNING_COLOR 0xF800   // Red
#define GAUGE_NORMAL_COLOR 0xFFFF    // White
#define GAUGE_ACCENT_COLOR 0x7BEF    // Light gray

// Gauge state
float currentPressure = 0.0;
float displayPressure = 0.0;  // For smooth animation
bool sweepComplete = false;
unsigned long lastUpdateTime = 0;

// Simulated data for testing
bool useSimulatedData = true;  // Set to false when using real sensor
float simulatedPressure = 0.0;
unsigned long lastSimUpdate = 0;

// Function prototypes
float readOilPressure();
float getSimulatedPressure();
void drawGauge(float pressure);
void drawNeedle(float pressure, uint16_t color);
void performStartupSweep();
uint16_t getPressureColor(float pressure);
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
void drawToyotaLogo(int x, int y, int size);
void redrawTickMarks();
void redrawLabels();

// Read oil pressure from sensor
float readOilPressure() {
  if (useSimulatedData) {
    return getSimulatedPressure();
  }

  // Read ADC value (average of 10 readings for stability)
  int adcSum = 0;
  for (int i = 0; i < 10; i++) {
    adcSum += analogRead(OIL_PRESSURE_PIN);
    delay(1);
  }
  float adcValue = adcSum / 10.0;

  // Convert ADC to voltage (ESP32 12-bit ADC, 3.3V reference)
  float measuredVoltage = adcValue * 3.3 / 4095.0;

  // Convert to actual sensor voltage (account for voltage divider)
  float sensorVoltage = measuredVoltage * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2;

  // Convert voltage to PSI (linear sensor: 0.5V = 0 PSI, 4.5V = 100 PSI)
  float pressure = (sensorVoltage - SENSOR_MIN_VOLTAGE) / (SENSOR_MAX_VOLTAGE - SENSOR_MIN_VOLTAGE) * SENSOR_MAX_PSI;

  // Clamp to valid range
  if (pressure < 0) pressure = 0;
  if (pressure > SENSOR_MAX_PSI) pressure = SENSOR_MAX_PSI;

  return pressure;
}

// Generate simulated oil pressure data
float getSimulatedPressure() {
  unsigned long currentTime = millis();

  // Simulate different pressure scenarios
  unsigned long runtime = currentTime / 1000; // seconds since startup

  if (runtime < 5) {
    // Cold start - higher pressure
    simulatedPressure = 55 + random(-3, 3);
  } else if (runtime < 15) {
    // Warming up - pressure dropping
    simulatedPressure = 55 - (runtime - 5) * 3 + random(-2, 2);
  } else if (runtime < 30) {
    // Normal idle - fluctuating around 15-20 psi
    simulatedPressure = 17 + sin(runtime * 0.5) * 3 + random(-1, 1);
  } else if (runtime < 35) {
    // Simulate rev up
    float revProgress = (runtime - 30) / 5.0;
    simulatedPressure = 17 + revProgress * 35 + random(-2, 2);
  } else if (runtime < 45) {
    // Cruising - steady 40-45 psi
    simulatedPressure = 42 + sin(runtime * 0.3) * 2 + random(-1, 1);
  } else if (runtime < 50) {
    // Rev down
    float revProgress = (runtime - 45) / 5.0;
    simulatedPressure = 52 - revProgress * 35 + random(-2, 2);
  } else {
    // Back to idle
    simulatedPressure = 17 + sin(runtime * 0.5) * 3 + random(-1, 1);
  }

  return simulatedPressure;
}

// Map float values (like Arduino map but for floats)
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Get color based on pressure value (1990s style - simpler)
uint16_t getPressureColor(float pressure) {
  if (pressure < OIL_PRESSURE_MIN_WARN) {
    return GAUGE_WARNING_COLOR;  // Red for low pressure
  } else {
    return GAUGE_NEEDLE_COLOR;  // Orange for normal (Toyota style)
  }
}

// Draw the gauge face (1990s Toyota MR2 SW20 style)
void drawGauge(float pressure) {
  // Draw outer ring (white bezel)
  tft.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, GAUGE_RADIUS, GAUGE_TEXT_COLOR);
  tft.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, GAUGE_RADIUS - 1, GAUGE_TEXT_COLOR);

  // Draw inner ring
  tft.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, GAUGE_RADIUS - 10, GAUGE_ACCENT_COLOR);

  int startAngle = GAUGE_START_ANGLE;
  int endAngle = GAUGE_END_ANGLE;

  // Draw red warning arc for low pressure zone (0-15 PSI)
  float warningEndAngle = mapFloat(OIL_PRESSURE_MIN_WARN, 0, 100, startAngle, endAngle);
  for (float angle = startAngle; angle <= warningEndAngle; angle += 1) {
    float angleRad = angle * PI / 180.0;
    int x1 = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 3);
    int y1 = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 3);
    int x2 = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 9);
    int y2 = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 9);
    tft.drawLine(x1, y1, x2, y2, GAUGE_WARNING_COLOR);
  }

  // Draw tick marks and numbers - 1990s Toyota style
  for (int psi = 0; psi <= 100; psi += 10) {
    float angle = mapFloat(psi, 0, 100, startAngle, endAngle);
    float angleRad = angle * PI / 180.0;

    // All tick marks are white in classic Toyota style
    uint16_t tickColor = GAUGE_TEXT_COLOR;

    // Draw major tick marks (bolder, like MR2 gauges)
    int x1 = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 12);
    int y1 = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 12);
    int x2 = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 22);
    int y2 = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 22);

    // Triple line for thickness (Toyota gauges had thick marks)
    tft.drawLine(x1, y1, x2, y2, tickColor);
    tft.drawLine(x1 + 1, y1, x2 + 1, y2, tickColor);
    tft.drawLine(x1 - 1, y1, x2 - 1, y2, tickColor);

    // Draw numbers every 20 psi (0, 20, 40, 60, 80, 100)
    if (psi % 20 == 0) {
      tft.setTextColor(GAUGE_TEXT_COLOR);
      tft.setTextSize(2);  // Larger, bolder numbers like MR2
      int numX = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 38);
      int numY = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 38);

      // Center the text
      String numStr = String(psi);
      int textWidth = numStr.length() * 12;  // Adjusted for size 2
      tft.setCursor(numX - textWidth / 2, numY - 8);
      tft.print(numStr);
    }

    // Draw minor tick marks (every 5 psi) - shorter and thinner
    if (psi < 100) {
      float minorAngle = mapFloat(psi + 5, 0, 100, startAngle, endAngle);
      float minorAngleRad = minorAngle * PI / 180.0;

      int mx1 = SCREEN_CENTER_X + cos(minorAngleRad) * (GAUGE_RADIUS - 12);
      int my1 = SCREEN_CENTER_Y + sin(minorAngleRad) * (GAUGE_RADIUS - 12);
      int mx2 = SCREEN_CENTER_X + cos(minorAngleRad) * (GAUGE_RADIUS - 18);
      int my2 = SCREEN_CENTER_Y + sin(minorAngleRad) * (GAUGE_RADIUS - 18);
      tft.drawLine(mx1, my1, mx2, my2, tickColor);
    }
  }

  // Draw "OIL" text at top (classic MR2 style - smaller, cleaner)
  tft.setTextColor(GAUGE_TEXT_COLOR);
  tft.setTextSize(1);
  tft.setCursor(100, 40);
  tft.print("OIL");

  // Draw "x100 PSI" below (like MR2 had "x100 km/h" on speedo)
  tft.setTextSize(1);
  tft.setCursor(88, 52);
  tft.print("x1 PSI");

  // Draw digital readout box (MR2 style - simple box at bottom)
  int boxX = 85;
  int boxY = 175;
  int boxW = 70;
  int boxH = 25;

  tft.drawRect(boxX, boxY, boxW, boxH, GAUGE_TEXT_COLOR);
  tft.fillRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, GAUGE_FACE_COLOR);

  tft.setTextSize(2);
  uint16_t valueColor = getPressureColor(pressure);
  tft.setTextColor(valueColor);
  String pressureStr = String((int)pressure);  // Whole numbers only, cleaner
  int digitWidth = pressureStr.length() * 12;
  tft.setCursor(120 - digitWidth / 2, 180);
  tft.print(pressureStr);

  // Draw center hub (classic Toyota style - simple)
  tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 6, GAUGE_ACCENT_COLOR);
  tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_FACE_COLOR);
  tft.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_TEXT_COLOR);
}

// Redraw tick marks (called after erasing needle to fix any erased marks)
void redrawTickMarks() {
  int startAngle = GAUGE_START_ANGLE;
  int endAngle = GAUGE_END_ANGLE;

  // Redraw tick marks
  for (int psi = 0; psi <= 100; psi += 10) {
    float angle = mapFloat(psi, 0, 100, startAngle, endAngle);
    float angleRad = angle * PI / 180.0;

    uint16_t tickColor = GAUGE_TEXT_COLOR;

    // Draw major tick marks
    int x1 = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 12);
    int y1 = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 12);
    int x2 = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 22);
    int y2 = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 22);

    tft.drawLine(x1, y1, x2, y2, tickColor);
    tft.drawLine(x1 + 1, y1, x2 + 1, y2, tickColor);
    tft.drawLine(x1 - 1, y1, x2 - 1, y2, tickColor);

    // Redraw numbers every 20 psi
    if (psi % 20 == 0) {
      tft.setTextColor(GAUGE_TEXT_COLOR);
      tft.setTextSize(2);
      int numX = SCREEN_CENTER_X + cos(angleRad) * (GAUGE_RADIUS - 38);
      int numY = SCREEN_CENTER_Y + sin(angleRad) * (GAUGE_RADIUS - 38);

      String numStr = String(psi);
      int textWidth = numStr.length() * 12;
      tft.setCursor(numX - textWidth / 2, numY - 8);
      tft.print(numStr);
    }

    // Redraw minor tick marks
    if (psi < 100) {
      float minorAngle = mapFloat(psi + 5, 0, 100, startAngle, endAngle);
      float minorAngleRad = minorAngle * PI / 180.0;

      int mx1 = SCREEN_CENTER_X + cos(minorAngleRad) * (GAUGE_RADIUS - 12);
      int my1 = SCREEN_CENTER_Y + sin(minorAngleRad) * (GAUGE_RADIUS - 12);
      int mx2 = SCREEN_CENTER_X + cos(minorAngleRad) * (GAUGE_RADIUS - 18);
      int my2 = SCREEN_CENTER_Y + sin(minorAngleRad) * (GAUGE_RADIUS - 18);
      tft.drawLine(mx1, my1, mx2, my2, tickColor);
    }
  }
}

// Redraw labels (OIL and x1 PSI text)
void redrawLabels() {
  tft.setTextColor(GAUGE_TEXT_COLOR);
  tft.setTextSize(1);
  tft.setCursor(100, 40);
  tft.print("OIL");

  tft.setTextSize(1);
  tft.setCursor(88, 52);
  tft.print("x1 PSI");
}

// Draw the needle at a specific pressure (1990s Toyota style - tapered)
void drawNeedle(float pressure, uint16_t color) {
  // Clamp pressure to valid range
  if (pressure < 0) pressure = 0;
  if (pressure > 100) pressure = 100;

  // Calculate angle
  float angle = mapFloat(pressure, 0, 100, GAUGE_START_ANGLE, GAUGE_END_ANGLE);
  float angleRad = angle * PI / 180.0;

  // Calculate needle end point
  int needleLength = GAUGE_RADIUS - 25;
  int x = SCREEN_CENTER_X + cos(angleRad) * needleLength;
  int y = SCREEN_CENTER_Y + sin(angleRad) * needleLength;

  // Draw tapered needle (classic Toyota style - wider at base)
  // Calculate perpendicular angle for needle width
  float perpAngle = angleRad + PI / 2.0;

  // Base width (wider at center)
  int baseWidth = 3;
  int x1 = SCREEN_CENTER_X + cos(perpAngle) * baseWidth;
  int y1 = SCREEN_CENTER_Y + sin(perpAngle) * baseWidth;
  int x2 = SCREEN_CENTER_X - cos(perpAngle) * baseWidth;
  int y2 = SCREEN_CENTER_Y - sin(perpAngle) * baseWidth;

  // If erasing (color is black), draw a much larger triangle to fully cover everything
  if (color == GAUGE_FACE_COLOR) {
    int baseWidthErase = 7;  // Much larger to erase all artifacts
    int x1e = SCREEN_CENTER_X + cos(perpAngle) * baseWidthErase;
    int y1e = SCREEN_CENTER_Y + sin(perpAngle) * baseWidthErase;
    int x2e = SCREEN_CENTER_X - cos(perpAngle) * baseWidthErase;
    int y2e = SCREEN_CENTER_Y - sin(perpAngle) * baseWidthErase;

    // Draw filled triangle
    tft.fillTriangle(x1e, y1e, x2e, y2e, x, y, color);
    // Also draw outline in black to ensure complete erasure
    tft.drawTriangle(x1e, y1e, x2e, y2e, x, y, color);
  } else {
    // Draw filled triangle needle WITHOUT outline to prevent artifacts
    tft.fillTriangle(x1, y1, x2, y2, x, y, color);
  }
}

// Draw MR2 startup text (1990s Toyota MR2 style)
void drawToyotaLogo(int centerX, int centerY, int scale) {
  // Draw "MR2" in large bold text (1990s Toyota MR2 aesthetic)
  tft.setTextColor(GAUGE_TEXT_COLOR);
  tft.setTextSize(5);

  // Calculate text position to center "MR2" (3 characters)
  // Each character is about 30 pixels wide at size 5
  int textWidth = 3 * 30;
  int textX = centerX - textWidth / 2;
  int textY = centerY - 20;  // Half of text height

  tft.setCursor(textX, textY);
  tft.print("MR2");
}

// Perform startup sweep animation (1990s Toyota style)
void performStartupSweep() {
  Serial.println("Performing startup sweep...");

  tft.fillScreen(GAUGE_FACE_COLOR);
  drawGauge(0);

  // Single sweep from 0 to max and back (classic Toyota startup)
  // Sweep up - smoother animation with smaller steps
  for (float i = 0; i <= 100; i += 2) {
    drawNeedle(i, GAUGE_NEEDLE_COLOR);
    delay(8);
    if (i < 100) {
      drawNeedle(i, GAUGE_FACE_COLOR);  // Erase

      // Redraw tick marks that may have been erased
      redrawTickMarks();

      // Redraw labels that may have been erased
      redrawLabels();

      // Redraw center hub
      tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 6, GAUGE_ACCENT_COLOR);
      tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_FACE_COLOR);
      tft.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_TEXT_COLOR);
    }
  }

  delay(150);

  // Sweep down - smoother animation with smaller steps
  for (float i = 100; i >= 0; i -= 2) {
    drawNeedle(i, GAUGE_NEEDLE_COLOR);
    delay(8);
    if (i > 0) {
      drawNeedle(i, GAUGE_FACE_COLOR);  // Erase

      // Redraw tick marks that may have been erased
      redrawTickMarks();

      // Redraw labels that may have been erased
      redrawLabels();

      // Redraw center hub
      tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 6, GAUGE_ACCENT_COLOR);
      tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_FACE_COLOR);
      tft.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_TEXT_COLOR);
    }
  }

  delay(200);

  Serial.println("Sweep complete!");
  sweepComplete = true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n2GR-FE Oil Pressure Gauge");
  Serial.println("=========================");

  // Configure ADC pin
  pinMode(OIL_PRESSURE_PIN, INPUT);
  analogReadResolution(12);  // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db);  // Full range 0-3.3V

  Serial.print("Initializing display...");
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(GAUGE_FACE_COLOR);
  Serial.println("Done!");

  // Show Toyota logo on startup
  drawToyotaLogo(120, 120, 10);
  delay(2000);

  // Perform startup sweep
  performStartupSweep();

  // Draw initial gauge
  tft.fillScreen(GAUGE_FACE_COLOR);
  currentPressure = readOilPressure();
  displayPressure = 0;
  drawGauge(displayPressure);

  lastUpdateTime = millis();

  if (useSimulatedData) {
    Serial.println("*** USING SIMULATED DATA ***");
    Serial.println("Set useSimulatedData = false for real sensor readings");
  }

  Serial.println("\nOil Pressure Specifications (2GR-FE):");
  Serial.println("  Minimum at idle (hot): 11.6 psi");
  Serial.println("  At 6000 rpm: 55.5+ psi");
  Serial.println("  Normal range: 15-60 psi\n");

  Serial.println("Sensor Configuration:");
  Serial.println("  Lowdoller Motorsports PN 7990100");
  Serial.println("  Range: 0-100 PSI (0.5V - 4.5V)");
  Serial.println("  Voltage divider: R1=3.9kΩ, R2=10kΩ");
  Serial.println("  ADC Pin: GPIO34\n");
}

void loop() {
  unsigned long currentTime = millis();

  // Read pressure every 100ms
  if (currentTime - lastUpdateTime >= 100) {
    lastUpdateTime = currentTime;

    // Read new pressure
    currentPressure = readOilPressure();

    // Smooth the display pressure (exponential moving average)
    float alpha = 0.15;  // Smoothing factor (0 = no change, 1 = instant)
    displayPressure = displayPressure * (1 - alpha) + currentPressure * alpha;

    // Only update if pressure changed enough to warrant redraw (reduce flashing)
    static float lastDisplayPressure = 0;
    if (abs(displayPressure - lastDisplayPressure) > 0.5) {
      // Erase old needle
      drawNeedle(lastDisplayPressure, GAUGE_FACE_COLOR);

      // Redraw tick marks that may have been erased
      redrawTickMarks();

      // Redraw labels that may have been erased
      redrawLabels();

      // Redraw gauge elements that might have been erased
      tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 6, GAUGE_ACCENT_COLOR);
      tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_FACE_COLOR);
      tft.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 4, GAUGE_TEXT_COLOR);

      // Draw new needle
      uint16_t needleColor = getPressureColor(displayPressure);
      drawNeedle(displayPressure, needleColor);

      lastDisplayPressure = displayPressure;
    }

    // Update digital readout
    int boxX = 86;
    int boxY = 176;
    int boxW = 68;
    int boxH = 23;
    tft.fillRect(boxX, boxY, boxW, boxH, GAUGE_FACE_COLOR);

    tft.setTextSize(2);
    uint16_t valueColor = getPressureColor(displayPressure);
    tft.setTextColor(valueColor);
    String pressureStr = String((int)displayPressure);  // Whole numbers
    int digitWidth = pressureStr.length() * 12;
    tft.setCursor(120 - digitWidth / 2, 180);
    tft.print(pressureStr);

    // Print to serial for debugging
    if (!useSimulatedData) {
      int adcValue = analogRead(OIL_PRESSURE_PIN);
      float voltage = adcValue * 3.3 / 4095.0;
      float sensorVoltage = voltage * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2;

      Serial.print("ADC: ");
      Serial.print(adcValue);
      Serial.print(" | Measured V: ");
      Serial.print(voltage, 3);
      Serial.print("V | Sensor V: ");
      Serial.print(sensorVoltage, 3);
      Serial.print("V | Pressure: ");
      Serial.print(displayPressure, 1);
      Serial.println(" PSI");
    } else {
      // Print simulated pressure every second
      static unsigned long lastPrint = 0;
      if (currentTime - lastPrint >= 1000) {
        lastPrint = currentTime;
        Serial.print("Simulated Oil Pressure: ");
        Serial.print(displayPressure, 1);
        Serial.print(" PSI");

        if (displayPressure < OIL_PRESSURE_MIN_SAFE) {
          Serial.println(" [CRITICAL LOW!]");
        } else if (displayPressure < OIL_PRESSURE_MIN_WARN) {
          Serial.println(" [WARNING - Low]");
        } else if (displayPressure <= OIL_PRESSURE_MAX_NORM) {
          Serial.println(" [NORMAL]");
        } else {
          Serial.println(" [HIGH - Cold/High RPM]");
        }
      }
    }
  }
}
