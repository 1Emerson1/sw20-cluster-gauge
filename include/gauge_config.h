#ifndef GAUGE_CONFIG_H
#define GAUGE_CONFIG_H

#include <IPAddress.h>

// NVS namespace
#define NVS_NAMESPACE "gauge_cfg"

// --- Default values (matching original #defines) ---
#define DEFAULT_USE_SIMULATED_DATA      true
#define DEFAULT_USE_SIMULATED_TEMP      true
#define DEFAULT_USE_SIMULATED_HEADLIGHT true

#define DEFAULT_SENSOR_MIN_VOLTAGE  0.5f
#define DEFAULT_SENSOR_MAX_VOLTAGE  4.5f
#define DEFAULT_SENSOR_MAX_PSI      100.0f
#define DEFAULT_VOLTAGE_DIVIDER_R1  3900.0f
#define DEFAULT_VOLTAGE_DIVIDER_R2  10000.0f

#define DEFAULT_OIL_PRESSURE_MIN_SAFE  8.0f
#define DEFAULT_OIL_PRESSURE_MIN_WARN  10.0f
#define DEFAULT_TEMP_WARNING_HIGH      110.0f

#define DEFAULT_BL_BRIGHTNESS_DAY   255
#define DEFAULT_BL_BRIGHTNESS_NIGHT 80
#define DEFAULT_BL_FADE_DURATION    500

#define DEFAULT_EMA_ALPHA           0.15f

// WiFi AP settings
#define WIFI_AP_SSID     "SW20-Gauge"
#define WIFI_AP_PASSWORD "mr2gauge1"
#define WIFI_AP_CHANNEL  1
#define WIFI_AP_MAX_CONN 2

// Runtime configuration structure
struct GaugeConfig {
    // Simulation toggles
    bool useSimulatedData;
    bool useSimulatedTemp;
    bool useSimulatedHeadlight;

    // Sensor calibration
    float sensorMinVoltage;
    float sensorMaxVoltage;
    float sensorMaxPsi;
    float voltageDividerR1;
    float voltageDividerR2;

    // Safety thresholds
    float oilPressureMinSafe;
    float oilPressureMinWarn;
    float tempWarningHigh;

    // Backlight
    int blBrightnessDay;
    int blBrightnessNight;
    int blFadeDuration;

    // Display
    float emaAlpha;
};

// NVS key names (max 15 chars for Preferences.h)
#define KEY_SIM_DATA    "simData"
#define KEY_SIM_TEMP    "simTemp"
#define KEY_SIM_HL      "simHL"
#define KEY_SENS_MIN_V  "sensMinV"
#define KEY_SENS_MAX_V  "sensMaxV"
#define KEY_SENS_MAX_P  "sensMaxP"
#define KEY_VD_R1       "vdR1"
#define KEY_VD_R2       "vdR2"
#define KEY_OIL_SAFE    "oilSafe"
#define KEY_OIL_WARN    "oilWarn"
#define KEY_TEMP_WARN   "tempWarn"
#define KEY_BL_DAY      "blDay"
#define KEY_BL_NIGHT    "blNight"
#define KEY_BL_FADE     "blFade"
#define KEY_EMA_ALPHA   "emaAlpha"

#endif // GAUGE_CONFIG_H
