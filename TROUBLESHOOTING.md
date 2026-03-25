- Issue: Flash upload fails
- Fix: verify USB cable/data, retry `pio run -e m5stickc_plus2 -t upload`
- Fix: check serial port selection in PlatformIO

- Issue: Device not detected
- Fix: reconnect USB, try different port, check OS driver
- Fix: run `pio device list` and target the listed port

- Issue: Boot loop or crash on startup
- Fix: monitor logs via `pio device monitor -b 115200`
- Fix: revert recent changes in `src/main.ino` or `src/app/state.*`
- Fix: confirm `SPIFFS.begin(true)` succeeds and assets are uploaded

- Issue: SPIFFS mount failed
- Fix: run `pio run -e m5stickc_plus2 -t uploadfs`

- Issue: SD card not mounted
- Fix: set correct CS pin in Settings -> SD CS Pin
- Fix: confirm SD wiring or slot seating

- Issue: IR send not working
- Fix: set correct IR pin in Settings -> IR Pin
- Fix: verify IR LED hardware and power

- Issue: Hardware mismatch (ASSUMPTION: using esp32s3-handheld PCB with this firmware)
- Fix: align pins in `src/app/state.h` and `src/modules/system/config.*` with `hardware/esp32s3-handheld/gpio_map.txt`
