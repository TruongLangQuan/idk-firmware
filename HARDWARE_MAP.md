- TARGET BOARD: M5StickC Plus2 (ASSUMPTION; inferred from `platformio.ini` + `src/main.ino` comment)
- SECTION: Runtime Hardware (firmware in `src/`)
- Component: Display + buttons via M5Unified
- Files: `src/core/*`, `src/screens/*`, `src/modules/media/*`, `src/modules/features/*`
- Pins: ASSUMPTION (managed by M5Unified)
- Protocol: ASSUMPTION (board-internal)
- Risk: HIGH

- Component: Backlight PWM
- Files: `src/system/power.cpp`, `src/app/state.h`
- Pins: BACKLIGHT_PIN=27
- Protocol: PWM/LEDC
- Risk: HIGH

- Component: IR transmitter
- Files: `src/modules/features/ir.*`, `src/modules/system/config.*`, `src/screens/ir.*`
- Pins: IR_PIN default 19, runtime configurable
- Protocol: IR carrier (IRremoteESP8266)
- Risk: HIGH

- Component: SD card
- Files: `src/screens/files.cpp`, `src/modules/system/backup.cpp`, `src/modules/network/webui.cpp`, `src/modules/system/config.*`
- Pins: SD CS configurable (default 4)
- Protocol: SPI (SD library)
- Risk: HIGH

- Component: SPIFFS/LittleFS
- Files: `src/main.ino`, `src/modules/media/*`, `src/modules/system/backup.cpp`, `src/modules/network/webui.cpp`
- Pins: N/A
- Protocol: internal flash FS
- Risk: MEDIUM

- Component: WiFi + WebServer
- Files: `src/modules/network/*`, `src/core/ui.cpp`
- Pins: N/A
- Protocol: 802.11 + HTTP
- Risk: MEDIUM

- Component: Power/Battery
- Files: `src/core/ui.cpp`, `src/screens/setting.cpp`, `src/main.ino`
- Pins: ASSUMPTION (managed by M5Unified)
- Protocol: ASSUMPTION
- Risk: MEDIUM

- SECTION: PCB Project `hardware/esp32s3-handheld` (ASSUMPTION: separate board, not referenced by firmware)
- Source: `hardware/esp32s3-handheld/gpio_map.txt`

- Component: ESP32-S3-WROOM-2 module
- Files: `hardware/esp32s3-handheld/esp32s3-handheld.kicad_*`
- Pins: see gpio_map
- Protocol: multi
- Risk: HIGH

- Component: LCD ST7789
- Pins: SCLK 39, MOSI 40, CS 41, DC 42, RST 21, BL 18
- Protocol: SPI
- Risk: HIGH

- Component: MicroSD
- Pins: CS 10, MOSI 11, MISO 13, SCLK 12
- Protocol: SPI
- Risk: HIGH

- Component: I2C/RTC
- Pins: SDA 16, SCL 17
- Protocol: I2C
- Risk: HIGH

- Component: CH9329 UART
- Pins: TX 43, RX 44
- Protocol: UART
- Risk: HIGH

- Component: IR receiver + transmitter
- Pins: RX 9, TX 46
- Protocol: IR
- Risk: HIGH

- Component: RGB LED
- Pins: GPIO38
- Protocol: GPIO/PWM
- Risk: HIGH

- Component: Buttons + BOOT
- Pins: BTN1 4, BTN2 5, BTN3 6, BTN4 7, BTN5 8, BOOT 0
- Protocol: GPIO
- Risk: HIGH

- Component: USB D+/D-
- Pins: D+ 20, D- 19
- Protocol: USB
- Risk: HIGH

- Component: External SPI header
- Pins: SD SPI shared, EXT_IRQ 15
- Protocol: SPI + GPIO
- Risk: HIGH
