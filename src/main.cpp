#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <lvgl.h>


// Create display object (pins configured in platformio.ini)
TFT_eSPI tft = TFT_eSPI();

// Oil pressure sensor configuration
#define OIL_PRESSURE_PIN 3  // GPIO3 - ADC1_CH2

// Voltage divider resistors (to scale 4.5V to 3.3V max)
#define VOLTAGE_DIVIDER_R1 3900.0  // Ohms
#define VOLTAGE_DIVIDER_R2 10000.0 // Ohms

// Sensor voltage range (Lowdoller 7990100 specifications)
#define SENSOR_MIN_VOLTAGE 0.5  // 0 PSI
#define SENSOR_MAX_VOLTAGE 4.5  // 100 PSI
#define SENSOR_MAX_PSI 100.0

// 2GR-FE oil pressure specifications
#define OIL_PRESSURE_MIN_SAFE 8.0
#define OIL_PRESSURE_MIN_WARN 10.0

// Temperature thresholds
#define TEMP_WARNING_HIGH 110.0

// Headlight backlight dimming configuration
#define HEADLIGHT_PIN 14
#define BL_PIN 40
#define BL_BRIGHTNESS_DAY 255
#define BL_BRIGHTNESS_NIGHT 80
#define BL_FADE_DURATION 500
#define BL_PWM_CHANNEL 0
#define BL_PWM_FREQ 5000
#define BL_PWM_RESOLUTION 8

// Display configuration
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// White-on-black color scheme
#define COLOR_ACCENT     lv_color_white()
#define COLOR_WHITE      lv_color_white()
#define COLOR_BLACK      lv_color_black()
#define COLOR_GREY       lv_color_hex(0x606060)
#define COLOR_NEEDLE     lv_color_white()
#define COLOR_WARNING    lv_color_hex(0xFF0000)

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 10];
static lv_color_t buf2[SCREEN_WIDTH * 10];
static lv_disp_drv_t disp_drv;

// LVGL objects
static lv_obj_t *meter;
static lv_meter_indicator_t *needle_temp;
static lv_obj_t *label_oil_temp;
static lv_obj_t *label_oil_press;
static lv_obj_t *label_press_val;
static lv_obj_t *label_press_unit;

// Gauge state
float currentPressure = 0.0;
float displayPressure = 0.0;
float currentTemp = 0.0;
float displayTemp = 0.0;
unsigned long lastUpdateTime = 0;

// Simulated data for testing
bool useSimulatedData = true;
bool useSimulatedTemp = true;
float simulatedPressure = 0.0;
float simulatedTemp = 0.0;

// Backlight fade state
bool useSimulatedHeadlight = true;
int currentBrightness = BL_BRIGHTNESS_DAY;
int targetBrightness = BL_BRIGHTNESS_DAY;
int fadeStartBrightness = BL_BRIGHTNESS_DAY;
unsigned long fadeStartTime = 0;
bool lastHeadlightState = false;

// Function prototypes
float readOilPressure();
float readCoolantTemp();
float getSimulatedPressure();
float getSimulatedTemp();
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void createGauge();
void updateGauge(float pressure, float temp);
void performStartup();
void updateBacklight();

// LVGL display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
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

// Read coolant temperature (placeholder for real sensor)
float readCoolantTemp() {
  if (useSimulatedTemp) {
    return getSimulatedTemp();
  }
  return 0.0;
}

// Generate simulated oil pressure data (2GR-FE realistic values)
float getSimulatedPressure() {
  unsigned long runtime = millis() / 1000;

  if (runtime < 5) {
    simulatedPressure = 60 + random(-3, 3);
  } else if (runtime < 15) {
    simulatedPressure = 60 - (runtime - 5) * 4.5 + random(-2, 2);
  } else if (runtime < 30) {
    simulatedPressure = 11 + sin(runtime * 0.5) * 2 + random(-1, 1);
  } else if (runtime < 35) {
    float revProgress = (runtime - 30) / 5.0;
    simulatedPressure = 11 + revProgress * 40 + random(-2, 2);
  } else if (runtime < 45) {
    simulatedPressure = 48 + sin(runtime * 0.3) * 3 + random(-1, 1);
  } else if (runtime < 50) {
    float revProgress = (runtime - 45) / 5.0;
    simulatedPressure = 48 - revProgress * 37 + random(-2, 2);
  } else {
    simulatedPressure = 11 + sin(runtime * 0.5) * 2 + random(-1, 1);
  }

  return simulatedPressure;
}

// Generate simulated coolant temperature (60-120°C range, 2GR-FE)
// Thermostat opens ~82°C, normal operating 85-95°C
float getSimulatedTemp() {
  unsigned long runtime = millis() / 1000;

  if (runtime < 10) {
    // Cold start warming up from 60°C
    simulatedTemp = 60 + runtime * 2.5 + random(-1, 1);
  } else if (runtime < 25) {
    // Warming through thermostat range
    float progress = (runtime - 10) / 15.0;
    simulatedTemp = 85 + progress * 5 + sin(runtime * 0.2) * 2 + random(-1, 1);
  } else if (runtime < 40) {
    // Normal operating temp ~90°C
    simulatedTemp = 90 + sin(runtime * 0.15) * 3 + random(-1, 1);
  } else if (runtime < 45) {
    // Brief spike (hard driving)
    float spike = sin((runtime - 40) * 0.628) * 12;
    simulatedTemp = 92 + spike + random(-1, 1);
  } else {
    // Back to normal
    simulatedTemp = 90 + sin(runtime * 0.15) * 3 + random(-1, 1);
  }

  if (simulatedTemp < 60) simulatedTemp = 60;
  if (simulatedTemp > 120) simulatedTemp = 120;

  return simulatedTemp;
}

// Create gauge: temperature arc with needle + digital oil pressure readout
void createGauge() {
  // --- Meter widget (temperature arc gauge) ---
  meter = lv_meter_create(lv_scr_act());
  lv_obj_set_size(meter, 232, 232);
  lv_obj_align(meter, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(meter, COLOR_BLACK, 0);
  lv_obj_set_style_bg_opa(meter, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(meter, 0, 0);
  lv_obj_set_style_pad_all(meter, 4, 0);
  lv_obj_set_style_text_font(meter, &lv_font_montserrat_16, LV_PART_TICKS);

  // Temperature scale: 100-260 deg F (2GR-FE oil temp range), 240 deg arc
  // LVGL rotation: 0 deg = 3 o'clock, clockwise. 8 o'clock = 150 deg.
  // 17 ticks (every 10 deg F), major every 4th (every 40 deg F)
  // Labels: 100, 140, 180, 220, 260
  lv_meter_scale_t *scale = lv_meter_add_scale(meter);
  lv_meter_set_scale_ticks(meter, scale, 17, 2, 10, COLOR_WHITE);
  lv_meter_set_scale_major_ticks(meter, scale, 4, 3, 16, COLOR_WHITE, 18);
  lv_meter_set_scale_range(meter, scale, 100, 260, 240, 150);

  // Red needle from center to tick edge
  needle_temp = lv_meter_add_needle_line(meter, scale, 3, COLOR_WARNING, -4);
  lv_meter_set_indicator_value(meter, needle_temp, 100);

  // Red center pivot dot
  lv_obj_set_style_size(meter, 12, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(meter, COLOR_WARNING, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(meter, LV_OPA_COVER, LV_PART_INDICATOR);

  // "OIL TEMP" label (inside gauge, upper area)
  label_oil_temp = lv_label_create(lv_scr_act());
  lv_label_set_text(label_oil_temp, "TEMP");
  lv_obj_set_style_text_font(label_oil_temp, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(label_oil_temp, COLOR_WHITE, 0);
  lv_obj_align(label_oil_temp, LV_ALIGN_CENTER, 0, -35);

  // "OIL PRESS" label (below center)
  label_oil_press = lv_label_create(lv_scr_act());
  lv_label_set_text(label_oil_press, "PRESSURE");
  lv_obj_set_style_text_font(label_oil_press, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(label_oil_press, COLOR_WHITE, 0);
  lv_obj_align(label_oil_press, LV_ALIGN_CENTER, 0, 36);

  // Pressure value (large digits)
  label_press_val = lv_label_create(lv_scr_act());
  lv_label_set_text(label_press_val, "0");
  lv_obj_set_style_text_font(label_press_val, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label_press_val, COLOR_WHITE, 0);
  lv_obj_align(label_press_val, LV_ALIGN_CENTER, 0, 68);

  // "PSI" unit label
  label_press_unit = lv_label_create(lv_scr_act());
  lv_label_set_text(label_press_unit, "PSI");
  lv_obj_set_style_text_font(label_press_unit, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(label_press_unit, COLOR_WHITE, 0);
  lv_obj_align(label_press_unit, LV_ALIGN_CENTER, 0, 100);
}

// Update gauge indicators
void updateGauge(float pressure, float temp) {
  // Convert temperature from C to F and clamp to scale range
  int tempF = (int)(temp * 9.0 / 5.0 + 32.0);
  if (tempF < 100) tempF = 100;
  if (tempF > 260) tempF = 260;
  lv_meter_set_indicator_value(meter, needle_temp, tempF);

  // Update pressure digital readout
  lv_label_set_text_fmt(label_press_val, "%d", (int)pressure);
  lv_obj_align(label_press_val, LV_ALIGN_CENTER, 0, 68);
}

// Startup sequence
void performStartup() {
  lv_obj_t *logo = lv_label_create(lv_scr_act());
  lv_label_set_text(logo, "MR2");
  lv_obj_set_style_text_color(logo, COLOR_WHITE, 0);
  lv_obj_set_style_text_font(logo, &lv_font_montserrat_48, 0);
  lv_obj_center(logo);

  for (int i = 0; i < 50; i++) {
    lv_timer_handler();
    delay(10);
  }

  lv_obj_del(logo);
  createGauge();
  lv_timer_handler();
}

// Update backlight based on headlight state with fade transition
void updateBacklight() {
  bool headlightOn;

  if (useSimulatedHeadlight) {
    headlightOn = ((millis() / 10000) % 2) == 1;
  } else {
    headlightOn = digitalRead(HEADLIGHT_PIN) == HIGH;
  }

  if (headlightOn != lastHeadlightState) {
    lastHeadlightState = headlightOn;
    fadeStartBrightness = currentBrightness;
    targetBrightness = headlightOn ? BL_BRIGHTNESS_NIGHT : BL_BRIGHTNESS_DAY;
    fadeStartTime = millis();
    Serial.print("Headlights ");
    Serial.println(headlightOn ? "ON - dimming" : "OFF - brightening");
  }

  if (currentBrightness != targetBrightness) {
    unsigned long elapsed = millis() - fadeStartTime;
    if (elapsed >= BL_FADE_DURATION) {
      currentBrightness = targetBrightness;
    } else {
      float progress = (float)elapsed / BL_FADE_DURATION;
      currentBrightness = fadeStartBrightness + (int)((targetBrightness - fadeStartBrightness) * progress);
    }
    ledcWrite(BL_PWM_CHANNEL, currentBrightness);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n\n2GR-FE Dual Gauge (Oil + Temp)");
  Serial.println("==============================");

  pinMode(OIL_PRESSURE_PIN, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  tft.init();
  tft.setRotation(0);
  // GC9A01 MADCTL: TFT_eSPI sets 0x08 (BGR) for rotation 0.
  // Clear bit 3 to use RGB order so LVGL colors render correctly.
  tft.writecommand(0x36);
  tft.writedata(0x00);  // MADCTL: RGB order (not BGR)
  tft.writecommand(0x3A);
  tft.writedata(0x55);  // RGB565
  tft.fillScreen(TFT_BLACK);

  // Headlight input
  pinMode(HEADLIGHT_PIN, INPUT_PULLDOWN);

  bool headlightsOnAtBoot;
  if (useSimulatedHeadlight) {
    headlightsOnAtBoot = ((millis() / 10000) % 2) == 1;
  } else {
    headlightsOnAtBoot = digitalRead(HEADLIGHT_PIN) == HIGH;
  }
  lastHeadlightState = headlightsOnAtBoot;
  int initialBrightness = headlightsOnAtBoot ? BL_BRIGHTNESS_NIGHT : BL_BRIGHTNESS_DAY;
  currentBrightness = initialBrightness;
  targetBrightness = initialBrightness;
  fadeStartBrightness = initialBrightness;

  ledcSetup(BL_PWM_CHANNEL, BL_PWM_FREQ, BL_PWM_RESOLUTION);
  ledcAttachPin(BL_PIN, BL_PWM_CHANNEL);
  ledcWrite(BL_PWM_CHANNEL, initialBrightness);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_WIDTH * 10);
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  lv_obj_set_style_bg_color(lv_scr_act(), COLOR_BLACK, 0);

  performStartup();

  lastUpdateTime = millis();

  if (useSimulatedData) Serial.println("*** SIMULATED OIL PRESSURE ***");
  if (useSimulatedTemp) Serial.println("*** SIMULATED TEMPERATURE ***");
}

void loop() {
  static unsigned long last_tick = 0;
  unsigned long currentTime = millis();

  lv_tick_inc(currentTime - last_tick);
  last_tick = currentTime;

  lv_timer_handler();
  updateBacklight();

  if (currentTime - lastUpdateTime >= 100) {
    lastUpdateTime = currentTime;

    currentPressure = readOilPressure();
    currentTemp = readCoolantTemp();

    // EMA smoothing
    float alpha = 0.15;
    displayPressure = displayPressure * (1 - alpha) + currentPressure * alpha;
    displayTemp = displayTemp * (1 - alpha) + currentTemp * alpha;

    updateGauge(displayPressure, displayTemp);
    // Serial logging (1Hz)
    static unsigned long lastPrint = 0;
    if (currentTime - lastPrint >= 1000) {
      lastPrint = currentTime;
      Serial.print("Oil: ");
      Serial.print(displayPressure, 1);
      Serial.print(" PSI | Temp: ");
      Serial.print(displayTemp, 1);
      Serial.println(" C");
    }
  }

  delay(5);
}
