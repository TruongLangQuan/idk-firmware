- Task: Add new sensor (ASSUMPTION: external sensor hardware exists)
- Files: `src/modules/<new>/`, `src/app/state.*`, `src/screens/*`, `src/main.ino`
- Risk: HIGH

- Task: Change display layout or menu order
- Files: `src/core/ui.*`, `src/screens/menu.cpp`, `src/app/state.cpp`
- Risk: MEDIUM

- Task: Modify WiFi behavior (scan, auto-connect, WebUI start)
- Files: `src/modules/network/*`, `src/screens/wifi.cpp`, `src/screens/setting.cpp`
- Risk: MEDIUM

- Task: Debug boot issue / crash on startup
- Files: `src/main.ino`, `src/core/log.*`, `src/system/power.*`, `src/modules/system/config.*`
- Risk: HIGH
