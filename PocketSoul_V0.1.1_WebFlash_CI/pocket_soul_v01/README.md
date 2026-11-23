# Pocket Soul V0.1.1 — Pixel Pet Screen

Target: Waveshare ESP32-S3-Touch-AMOLED-2.16
Framework: ESP-IDF 5.5.5 or 6.0.2, LVGL 9 via Waveshare BSP

## Milestone
- BSP display initialization based on the official board examples
- LVGL scene with pixel-block pet placeholder sprite
- Pet tap, Feed and Play interactions
- Mood/status labels
- Simple blink timer

## Build
Open an ESP-IDF terminal, then from this project directory:

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```
Replace `COMx` with the board's serial port. On Linux, use `/dev/ttyACM0` or the port reported by your system.

The managed BSP dependency is declared in `main/idf_component.yml`. The first build needs network access to ESP Component Registry.


## Important verification note
This project was created in an environment without GitHub network access or the target board. The C++ pet-core smoke test can be run locally, but the ESP-IDF/LVGL hardware project has **not** been built or flashed here. Please treat this milestone as integration-ready source, not a verified firmware binary. If a compile error appears, send the complete terminal log and we will fix it against the installed BSP version.
