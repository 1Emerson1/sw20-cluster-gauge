#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <lvgl.h>

// Create display object (pins configured in platformio.ini)
TFT_eSPI tft = TFT_eSPI();

// Oil pressure sensor configuration
#define OIL_PRESSURE_PIN 34  // GPIO34 - ADC1_CH6 (input only pin, good for ADC)

// Voltage divider resistors (to scale 4.5V to 3.3V max)
#define VOLTAGE_DIVIDER_R1 3900.0  // Ohms
#define VOLTAGE_DIVIDER_R2 10000.0 // Ohms

// Sensor voltage range (Lowdoller 7990100 specifications)
#define SENSOR_MIN_VOLTAGE 0.5  // 0 PSI
#define SENSOR_MAX_VOLTAGE 4.5  // 100 PSI (sensor max, but we'll scale to 80)
#define SENSOR_MAX_PSI 100.0

// 2GR-FE oil pressure specifications
#define OIL_PRESSURE_MIN_SAFE 8.0    // Below this = critical low
#define OIL_PRESSURE_MIN_WARN 10.0   // Below this = warning (can idle at 10 PSI)
#define OIL_PRESSURE_MAX_NORM 80.0   // Gauge maximum (high rpm/cold)

// Display configuration
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// 1990s Toyota MR2 color scheme - White Edition
#define GAUGE_FACE_COLOR lv_color_black()         // Black
#define GAUGE_TEXT_COLOR lv_color_white()         // White
#define GAUGE_NEEDLE_COLOR lv_color_white()       // White
#define GAUGE_WARNING_COLOR lv_color_hex(0xF800)  // Red (low pressure)
#define GAUGE_HIGH_WARN_COLOR lv_color_hex(0xFFE0) // Yellow (high pressure - RGB565)
#define GAUGE_ACCENT_COLOR lv_color_white()       // White

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 10];
static lv_color_t buf2[SCREEN_WIDTH * 10];
static lv_disp_drv_t disp_drv;

// LVGL objects
static lv_obj_t *meter;
static lv_meter_indicator_t *indic_needle;
static lv_obj_t *label_digital;
static lv_obj_t *label_oil;
static lv_obj_t *label_psi;

// Gauge state
float currentPressure = 0.0;
float displayPressure = 0.0;
unsigned long lastUpdateTime = 0;

// Simulated data for testing
bool useSimulatedData = true;  // Set to false when using real sensor
float simulatedPressure = 0.0;

// Function prototypes
float readOilPressure();
float getSimulatedPressure();
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void createGauge();
void updateGauge(float pressure);
void performStartupSweep();

// LVGL display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  // Cast LVGL color buffer directly to uint16_t array for TFT_eSPI
  tft.pushColors((uint16_t *)color_p, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// Read oil pressure from sensor
float readOilPressure() {
  if (useSimulatedData) {
    return getSimulatedPressure();
  }

  int adcSum = 0;
  for (int i = 0; i < 10; i++) {
    adcSum += analogRead(OIL_PRESSURE_PIN);
    delay(1);
  }
  float adcValue = adcSum / 10.0;
  float measuredVoltage = adcValue * 3.3 / 4095.0;
  float sensorVoltage = measuredVoltage * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2;
  float pressure = (sensorVoltage - SENSOR_MIN_VOLTAGE) / (SENSOR_MAX_VOLTAGE - SENSOR_MIN_VOLTAGE) * SENSOR_MAX_PSI;

  if (pressure < 0) pressure = 0;
  if (pressure > SENSOR_MAX_PSI) pressure = SENSOR_MAX_PSI;

  return pressure;
}

// Generate simulated oil pressure data (2GR-FE realistic values: 8-80 PSI)
float getSimulatedPressure() {
  unsigned long currentTime = millis();
  unsigned long runtime = currentTime / 1000;

  if (runtime < 5) {
    // Cold start - high pressure
    simulatedPressure = 60 + random(-3, 3);
  } else if (runtime < 15) {
    // Warming up - pressure dropping to idle
    simulatedPressure = 60 - (runtime - 5) * 4.5 + random(-2, 2);
  } else if (runtime < 30) {
    // Hot idle - normal fluctuation around 10-12 PSI
    simulatedPressure = 11 + sin(runtime * 0.5) * 2 + random(-1, 1);
  } else if (runtime < 35) {
    // Rev up - increasing pressure
    float revProgress = (runtime - 30) / 5.0;
    simulatedPressure = 11 + revProgress * 40 + random(-2, 2);
  } else if (runtime < 45) {
    // Cruising/high RPM - stable around 45-50 PSI
    simulatedPressure = 48 + sin(runtime * 0.3) * 3 + random(-1, 1);
  } else if (runtime < 50) {
    // Rev down - back to idle
    float revProgress = (runtime - 45) / 5.0;
    simulatedPressure = 48 - revProgress * 37 + random(-2, 2);
  } else {
    // Back to hot idle
    simulatedPressure = 11 + sin(runtime * 0.5) * 2 + random(-1, 1);
  }

  return simulatedPressure;
}

// Create the gauge with 1990s Toyota MR2 styling
void createGauge() {
  // Create meter widget - MAXIMIZED for 1.28" screen
  meter = lv_meter_create(lv_scr_act());
  lv_obj_set_size(meter, 230, 230);  // Nearly full screen
  lv_obj_align(meter, LV_ALIGN_CENTER, 0, -5);  // Slightly up for balance

  // Black background with white border (classic MR2)
  lv_obj_set_style_bg_color(meter, GAUGE_FACE_COLOR, 0);
  lv_obj_set_style_border_width(meter, 2, 0);
  lv_obj_set_style_border_color(meter, GAUGE_TEXT_COLOR, 0);
  lv_obj_set_style_pad_all(meter, 10, 0);

  // Add scale (0-80 PSI, 270° sweep) with ticks every 5 PSI
  lv_meter_scale_t *scale = lv_meter_add_scale(meter);
  lv_meter_set_scale_ticks(meter, scale, 17, 2, 10, GAUGE_TEXT_COLOR);  // 17 ticks (every 5 PSI: 0-80)
  lv_meter_set_scale_major_ticks(meter, scale, 4, 3, 15, GAUGE_TEXT_COLOR, 14);  // Major tick every 20 PSI
  lv_meter_set_scale_range(meter, scale, 0, 80, 270, 135);

  // Red warning arc (0-10 PSI - LOW pressure)
  lv_meter_indicator_t *indic_arc_low = lv_meter_add_arc(meter, scale, 6, GAUGE_WARNING_COLOR, 0);
  lv_meter_set_indicator_start_value(meter, indic_arc_low, 0);
  lv_meter_set_indicator_end_value(meter, indic_arc_low, OIL_PRESSURE_MIN_WARN);

  // Yellow warning arc (70-80 PSI - HIGH pressure)
  lv_meter_indicator_t *indic_arc_high = lv_meter_add_arc(meter, scale, 6, GAUGE_HIGH_WARN_COLOR, 0);
  lv_meter_set_indicator_start_value(meter, indic_arc_high, 70);
  lv_meter_set_indicator_end_value(meter, indic_arc_high, 80);

  // Thin cyan needle for modern look
  indic_needle = lv_meter_add_needle_line(meter, scale, 3, GAUGE_NEEDLE_COLOR, -10);

  // Add center circle for needle pivot
  lv_obj_t *center_circle = lv_obj_create(lv_scr_act());
  lv_obj_set_size(center_circle, 16, 16);
  lv_obj_set_style_radius(center_circle, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_circle, GAUGE_TEXT_COLOR, 0);
  lv_obj_set_style_border_width(center_circle, 0, 0);
  lv_obj_align(center_circle, LV_ALIGN_CENTER, 0, -5);

  // Oil label above needle pivot point
  label_oil = lv_label_create(lv_scr_act());
  lv_label_set_text(label_oil, "Oil");
  lv_obj_set_style_text_color(label_oil, GAUGE_ACCENT_COLOR, 0);
  lv_obj_set_style_text_font(label_oil, &lv_font_montserrat_18, 0);
  lv_obj_align(label_oil, LV_ALIGN_CENTER, 0, -35);

  // "PSI" label underneath needle pivot point
  label_psi = lv_label_create(lv_scr_act());
  lv_label_set_text(label_psi, "PSI");
  lv_obj_set_style_text_color(label_psi, GAUGE_ACCENT_COLOR, 0);
  lv_obj_set_style_text_font(label_psi, &lv_font_montserrat_14, 0);
  lv_obj_align(label_psi, LV_ALIGN_CENTER, 0, 15);

  // Extra large digital readout - MAXIMUM VISIBILITY (no border)
  label_digital = lv_label_create(lv_scr_act());
  lv_label_set_text(label_digital, "0");
  lv_obj_set_style_text_color(label_digital, GAUGE_NEEDLE_COLOR, 0);
  lv_obj_set_style_text_font(label_digital, &lv_font_montserrat_48, 0);
  lv_obj_set_style_bg_color(label_digital, GAUGE_FACE_COLOR, 0);
  lv_obj_set_style_bg_opa(label_digital, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(label_digital, 0, 0);  // No border
  lv_obj_set_style_pad_all(label_digital, 8, 0);
  lv_obj_align(label_digital, LV_ALIGN_BOTTOM_MID, 0, -5);
}

// Update gauge with new pressure value
void updateGauge(float pressure) {
  lv_meter_set_indicator_value(meter, indic_needle, (int32_t)pressure);

  char buf[8];
  snprintf(buf, sizeof(buf), "%d", (int)pressure);
  lv_label_set_text(label_digital, buf);

  // Color coding for digital readout
  if (pressure < OIL_PRESSURE_MIN_WARN) {
    // Low pressure warning - RED
    lv_obj_set_style_text_color(label_digital, GAUGE_WARNING_COLOR, 0);
  } else if (pressure > 70) {
    // High pressure warning - YELLOW
    lv_obj_set_style_text_color(label_digital, GAUGE_HIGH_WARN_COLOR, 0);
  } else {
    // Normal pressure - WHITE
    lv_obj_set_style_text_color(label_digital, GAUGE_NEEDLE_COLOR, 0);
  }
}

// Perform startup sweep animation (1990s Toyota style)
void performStartupSweep() {
  Serial.println("Performing startup sweep...");

  // Create large MR2 logo in WHITE
  lv_obj_t *logo = lv_label_create(lv_scr_act());
  lv_label_set_text(logo, "MR2");
  lv_obj_set_style_text_color(logo, GAUGE_NEEDLE_COLOR, 0);
  lv_obj_set_style_text_font(logo, &lv_font_montserrat_48, 0);
  lv_obj_center(logo);

  // Display logo for 2 seconds
  for (int i = 0; i < 200; i++) {
    lv_timer_handler();
    delay(10);
  }

  // Remove logo
  lv_obj_del(logo);

  // Small delay before gauge appears
  delay(100);

  // Create the gauge
  createGauge();

  // Sweep up to 80
  for (int i = 0; i <= 80; i += 2) {
    lv_meter_set_indicator_value(meter, indic_needle, i);
    lv_timer_handler();
    delay(8);
  }

  delay(150);

  // Sweep back to 0
  for (int i = 80; i >= 0; i -= 2) {
    lv_meter_set_indicator_value(meter, indic_needle, i);
    lv_timer_handler();
    delay(8);
  }

  delay(200);

  Serial.println("Sweep complete!");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n2GR-FE Oil Pressure Gauge (LVGL)");
  Serial.println("==================================");

  pinMode(OIL_PRESSURE_PIN, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.print("Initializing display...");
  tft.init();
  tft.setRotation(0);

  // Explicitly set RGB565 color format (COLMOD command 0x3A)
  tft.writecommand(0x3A);
  tft.writedata(0x55);  // 16-bit RGB565

  tft.fillScreen(TFT_BLACK);
  Serial.println("Done!");

  Serial.print("Initializing LVGL...");
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_WIDTH * 10);
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  Serial.println("Done!");

  // Set black background
  lv_obj_set_style_bg_color(lv_scr_act(), GAUGE_FACE_COLOR, 0);

  // Perform startup: MR2 logo → sweep
  performStartupSweep();

  lastUpdateTime = millis();

  if (useSimulatedData) {
    Serial.println("*** USING SIMULATED DATA ***");
  }

  Serial.println("\nOil Pressure Specifications (2GR-FE):");
  Serial.println("  Critical low: < 8 psi");
  Serial.println("  Can idle as low as: 10 psi");
  Serial.println("  At 6000 rpm: 55-60 psi");
  Serial.println("  Gauge range: 0-80 psi\n");
}

void loop() {
  static unsigned long last_tick = 0;
  unsigned long currentTime = millis();

  // Update LVGL tick
  lv_tick_inc(currentTime - last_tick);
  last_tick = currentTime;

  // Update LVGL timer
  lv_timer_handler();

  // Read pressure every 100ms
  if (currentTime - lastUpdateTime >= 100) {
    lastUpdateTime = currentTime;

    currentPressure = readOilPressure();

    // Smooth the display pressure
    float alpha = 0.15;
    displayPressure = displayPressure * (1 - alpha) + currentPressure * alpha;

    // Update gauge
    updateGauge(displayPressure);

    // Serial output
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

  delay(5);
}
