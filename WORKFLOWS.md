- WORKFLOW: Build firmware
- Step: `pio run -e m5stickc_plus2`
- Step: fix compile errors before flashing

- WORKFLOW: Flash device
- Step: connect device via USB
- Step: `pio run -e m5stickc_plus2 -t upload`
- Step: `pio device monitor -b 115200`

- WORKFLOW: Upload filesystem assets
- Step: update `data/*`
- Step: `pio run -e m5stickc_plus2 -t uploadfs`
- Step: reboot device, verify content lists

- WORKFLOW: Debug hardware issues
- Step: monitor logs via `pio device monitor -b 115200`
- Step: verify SD mount from Settings -> SD CS Pin
- Step: verify IR send from IR screen
- Step: verify WiFi scan/connect from WiFi screen

- WORKFLOW: Add a new module
- Step: add code under `src/modules/<area>/`
- Step: add any state to `src/app/state.*`
- Step: add screen handler in `src/screens/`
- Step: register menu item in `src/app/state.cpp`
- Step: include + dispatch in `src/main.ino`
