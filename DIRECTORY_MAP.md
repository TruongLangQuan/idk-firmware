- Folder: `src/`
- Purpose: Arduino firmware source (entry + modules + UI)
- Key files: `src/main.ino`, `src/app/state.*`
- Edit level: EDIT WITH CAUTION

- Folder: `src/app/`
- Purpose: global state, constants, screen enums
- Key files: `src/app/state.h`, `src/app/state.cpp`
- Edit level: EDIT WITH CAUTION

- Folder: `src/core/`
- Purpose: UI primitives, input handling, logging
- Key files: `src/core/ui.*`, `src/core/input.*`, `src/core/log.*`
- Edit level: EDIT WITH CAUTION

- Folder: `src/modules/`
- Purpose: features + subsystems
- Key files: `src/modules/features/*`, `src/modules/network/*`, `src/modules/media/*`, `src/modules/system/*`, `src/modules/files/*`
- Edit level: EDIT WITH CAUTION

- Folder: `src/screens/`
- Purpose: per-screen UI state machines
- Key files: `src/screens/*.cpp`
- Edit level: SAFE TO EDIT

- Folder: `src/system/`
- Purpose: low-level hardware helpers
- Key files: `src/system/power.*`
- Edit level: DO NOT EDIT

- Folder: `hardware/`
- Purpose: PCB project (ESP32-S3 handheld)
- Key files: `hardware/esp32s3-handheld/*.kicad_*`, `hardware/esp32s3-handheld/gpio_map.txt`
- Edit level: DO NOT EDIT

- Folder: `data/`
- Purpose: SPIFFS assets (fonts/gifs/ir/txt)
- Key files: `data/fonts/*`, `data/gifs/*`, `data/ir/*`, `data/txt/*`
- Edit level: SAFE TO EDIT

- Folder: `ui/`
- Purpose: React UI mock
- Key files: `ui/M5StickCSelectUI.jsx`
- Edit level: SAFE TO EDIT

- Folder: `include/`
- Purpose: project include dir (empty)
- Key files: none
- Edit level: SAFE TO EDIT

- Folder: `lib/`
- Purpose: local libs (empty)
- Key files: none
- Edit level: SAFE TO EDIT

- Folder: `.pio/`
- Purpose: PlatformIO build output (generated)
- Key files: generated
- Edit level: DO NOT EDIT

- Files: `firmware_complete.bin`, `firmware_ota.bin`
- Purpose: generated firmware images
- Edit level: DO NOT EDIT

- Folder: `.blackbox/`, `.blackboxcli/`, `.continue/`, `.vscode/`
- Purpose: tooling config
- Edit level: SAFE TO EDIT
