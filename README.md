# SW20 Cluster Gauge - 2GR-FE Oil Pressure Display

Oil pressure gauge for a Toyota 2GR-FE swap, built with LVGL on a Waveshare ESP32-S3 with integrated 1.28" round LCD.

## Hardware

- **Board**: [Waveshare ESP32-S3-LCD-1.28](https://www.waveshare.com/esp32-s3-lcd-1.28.htm) (GC9A01 display built in)
- **Sensor**: Lowdoller Motorsports 7990100 (0-100 PSI, 0.5V-4.5V output)
- **Engine**: Toyota 2GR-FE 3.5L V6

## Display Wiring

The LCD is integrated on the Waveshare board -- no external display wiring needed.

| Function | GPIO |
|----------|------|
| LCD MOSI | 11 |
| LCD CLK | 10 |
| LCD CS | 9 |
| LCD DC | 8 |
| LCD RST | 12 |
| LCD BL | 40 |

SPI runs at 80MHz using HSPI port.

## Oil Pressure Sensor

The sensor outputs up to 4.5V but the ESP32-S3 ADC max is 3.3V. A voltage divider is required.

```
Sensor Output (0.5V - 4.5V)
         |
         R1 (3.9k)
         |
         +-------> GPIO3 (ADC)
         |
         R2 (10k)
         |
        GND
```

Max output: 4.5V x 10k / 13.9k = 3.24V (safe for ESP32-S3).

## Headlight Backlight Dimming

The display dims when headlights are turned on (night driving). A switched 12V headlight signal is passed through a voltage divider to GPIO14.

```
Headlight 12V (switched)
         |
         R1 (voltage divider to 3.3V)
         |
         +-------> GPIO14
         |
         R2
         |
        GND
```

- Headlights off: full brightness (255)
- Headlights on: dimmed (80)
- 500ms fade transition between states
- Correctly handles boot with headlights already on

Both the oil pressure sensor and headlight input have simulation modes for bench testing (set `useSimulatedData` and `useSimulatedHeadlight` to `true` in main.cpp).

## 2GR-FE Oil Pressure Specs

| Condition | PSI |
|-----------|-----|
| Min at idle (hot) | 11.6 |
| At 6000 RPM | 55.5+ |
| Normal operating | 15-60 |
| Cold start | 60-80 |

## Gauge Design

1990s Toyota MR2 White Edition theme -- black face, white everything else.

- 0-80 PSI range, 270 degree sweep
- Red arc: 0-10 PSI (critical low)
- Yellow arc: 70-80 PSI (high pressure)
- 48pt digital readout with color coding (white/red/yellow)
- Major ticks every 20 PSI, minor every 5 PSI
- MR2 logo splash on startup with needle sweep

## Building

```bash
pio run                          # build
pio run --target upload          # flash
pio device monitor               # serial monitor
```

## Switching to Real Sensors

In `src/main.cpp`, set these to `false`:

```cpp
bool useSimulatedData = false;       // line 67
bool useSimulatedHeadlight = false;  // line 71
```

## Calibration

1. Check voltage divider resistor values with a multimeter
2. Verify sensor reads ~0.5V at 0 PSI, ~4.5V at 100 PSI
3. Adjust `VOLTAGE_DIVIDER_R1` / `R2` in code if using different resistors

## References

- [Waveshare ESP32-S3-LCD-1.28 Wiki](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)
- [LVGL Graphics Library](https://github.com/lvgl/lvgl)
- [Lowdoller Motorsports Sensor](https://lowdoller-motorsports.com/products/0-100-psi-5v-pressure-sensor)

## License

MIT License
