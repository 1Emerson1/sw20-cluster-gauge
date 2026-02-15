# SW20 Cluster Gauge - 2GR-FE Oil Pressure Display

ESP32-based oil pressure gauge using LVGL on a GC9A01 1.28" circular display for Toyota 2GR-FE engine monitoring.

## Hardware Components

- **Microcontroller**: ESP-WROOM-32
- **Display**: GC9A01 1.28" Round LCD (240x240)
- **Sensor**: Lowdoller Motorsports Pressure Sensor (PN 7990100)
  - Range: 0-100 PSI
  - Output: 0.5V (0 PSI) to 4.5V (100 PSI)
  - Accuracy: ±1% full scale
- **Engine**: Toyota 2GR-FE 3.5L V6

## Display Wiring

| Display Pin | ESP32 GPIO | Function |
|-------------|------------|----------|
| SCL         | 15         | SPI Clock |
| SDA         | 13         | SPI MOSI |
| DC          | 21         | Data/Command |
| CS          | 17         | Chip Select |
| RST         | 22         | Reset |
| VCC         | 3.3V       | Power |
| GND         | GND        | Ground |

## Oil Pressure Sensor Wiring

### Voltage Divider Circuit

**CRITICAL**: The sensor outputs up to 4.5V, but the ESP32 ADC maximum is 3.3V. A voltage divider is required to prevent damage to the GPIO pin.

```
Sensor Output (0.5V - 4.5V)
         |
         R1 (3.9kΩ)
         |
         +-------> To ESP32 GPIO34 (ADC)
         |
         R2 (10kΩ)
         |
        GND
```

**Voltage Divider Calculation:**
- Vout = Vin × R2 / (R1 + R2)
- Max Vout = 4.5V × 10kΩ / 13.9kΩ = 3.24V ✓ (Safe for ESP32)

### Wiring Connections

| Sensor Pin | Connection |
|------------|------------|
| Signal Out | → R1 (3.9kΩ) → GPIO34 + R2 (10kΩ) → GND |
| VCC        | +5V (from sensor power supply) |
| GND        | Common Ground |

**Resistor Options:**
- R1: 3.9kΩ (or 3.6kΩ, 4.0kΩ)
- R2: 10kΩ

## 2GR-FE Oil Pressure Specifications

Based on Toyota service manual specifications:

| Condition | Pressure (PSI) |
|-----------|----------------|
| Minimum at idle (hot) | 11.6 |
| At 6000 RPM | 55.5+ |
| Normal operating range | 15-60 |
| Cold start | 60-80 |

### Gauge Display Design (1990s Toyota MR2 White Edition)

**Visual Theme:**
- Black background with white border
- White needle, text, and tick marks
- Red arc: 0-10 PSI (critical low)
- Yellow arc: 70-80 PSI (high pressure warning)
- 270° sweep (0-80 PSI range)

**Digital Readout Color Coding:**
- **WHITE** (10-70 PSI): Normal operating range
- **RED** (< 10 PSI): Critical low pressure
- **YELLOW** (> 70 PSI): High pressure (cold engine or high RPM)

## Features

- ✅ LVGL-based UI for smooth graphics
- ✅ 1990s Toyota MR2 White Edition styling (black/white theme)
- ✅ Startup sweep animation with MR2 logo
- ✅ Color-coded warning arcs (red: 0-10 PSI, yellow: 70-80 PSI)
- ✅ Smooth needle animation with exponential smoothing
- ✅ Large digital readout (48pt) with color coding
- ✅ 270-degree gauge sweep (0-80 PSI)
- ✅ Major tick marks every 20 PSI
- ✅ Minor tick marks every 5 PSI
- ✅ "Oil" label above center, "PSI" label below
- ✅ Real-time serial monitoring
- ✅ Simulated data mode for testing

## Software Configuration

### Testing with Simulated Data

The code includes a realistic simulation mode for testing without the physical sensor:

```cpp
bool useSimulatedData = true;  // Line 57 in main.cpp
```

The simulation includes:
- Cold start (high pressure ~60 PSI)
- Warm-up period (pressure dropping)
- Hot idle (~11 PSI with fluctuation around 10-12 PSI)
- Rev up/cruising (~45-50 PSI)
- Rev down back to idle

### Switching to Real Sensor

When ready to use the real sensor:

1. Wire the voltage divider circuit as shown above
2. Connect sensor signal to GPIO34
3. Change line 57 in main.cpp:
   ```cpp
   bool useSimulatedData = false;
   ```
4. Upload and monitor serial output for voltage/pressure readings

## Building and Uploading

```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor

# Or combine upload and monitor
pio run --target upload && pio device monitor
```

## Serial Monitor Output

When testing, the serial monitor displays:

**Simulated Mode:**
```
Simulated Oil Pressure: 17.3 PSI [NORMAL]
Simulated Oil Pressure: 18.1 PSI [NORMAL]
```

**Real Sensor Mode:**
```
ADC: 2456 | Measured V: 1.982V | Sensor V: 2.754V | Pressure: 56.4 PSI
ADC: 1234 | Measured V: 0.995V | Sensor V: 1.382V | Pressure: 22.1 PSI
```

## Calibration

If readings seem off:

1. **Check voltage divider resistor values** with multimeter
2. **Verify sensor voltage** at different pressures:
   - 0 PSI should read ~0.5V
   - 100 PSI should read ~4.5V
3. **Adjust voltage divider ratio** in code if using different resistors (lines 13-14)
4. **Check ADC attenuation** setting (line 280) - should be `ADC_11db` for 0-3.3V range

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Display not working | Check SPI wiring, verify 3.3V power |
| Readings always 0 | Check voltage divider, verify GPIO34 connection |
| Erratic readings | Add 0.1µF capacitor between sensor signal and GND |
| Pressure too high/low | Verify resistor values, check sensor voltage range |
| Needle jumps around | Increase smoothing factor (line 341) or add hardware filter |

## Future Enhancements

- [ ] Multiple gauge support (oil temp, coolant temp, boost, etc.)
- [ ] Configurable warning thresholds via web interface
- [ ] Data logging to SD card
- [ ] WiFi connectivity for remote monitoring
- [ ] OLED menu system for configuration
- [ ] CAN bus integration for OEM sensor data

## References

- [2GR-FE Oil Pressure Specifications](https://wilhelmraceworks.com/blog/2gr-oil-pressure)
- [Lowdoller Motorsports Sensor PN 7990100](https://lowdoller-motorsports.com/products/0-100-psi-5v-pressure-sensor)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)
- [LVGL Graphics Library](https://github.com/lvgl/lvgl)

## License

MIT License - Feel free to use and modify for your own projects.
