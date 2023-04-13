# Setup procedure

## Install and configure Arduino IDE
- Download and install the latest Arduino IDE
   - https://www.arduino.cc/en/software
- Start the Arduino IDE
- Open "File" → "Preferences" from the menu
- Add the following URL to "Additional boards manager URLs"
   - https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
- Press OK and close Preferences
- Open "Tools" → "Boards" → "Boards Manager" from the menu
- Enter "RP2040" in the search field of Boards Manager
- Select "Raspberry Pi Pico/RP2040 by Earle F. Philhower, III" and press the Install button
- Exit the Arduino IDE after the installation is complete

## Get source code
- open the repository where the source code is stored
   - https://github.com/msawahara/me56ps2-emulator-rp2040
- Press the green "Code" button and select "Download ZIP" to download the source code
- Expand the downloaded "me56ps2-emulator-rp2040-master.zip"
- Delete the last "-master" from the folder name and rename it to "me56ps2-emulator-rp2040"

## write in
- Open me56ps2-emulator-rp2040.ino in the downloaded folder
- Connect W5500-EVB-Pico to PC while pressing BOOTSEL button
   - Release the BOOTSEL button after connecting
- Open "Select Board" → "Select other board and port"
- Choice of BOARDS
   - Enter "W5500" in the BOARDS search field
   - Select "WIZnet W5500-EVB-Pico"
- Selection of PORTS
   - Check "Show all ports"
   - Select "UF2 Board UF2 Devices"
- Press OK
- Select "Tools" → "USB Stack" → "No USB" from the menu (skip this step if there is no display)
- Select "Sketch" → "Upload" from the menu to start writing
- When "Done Uploading." or "Wrote xxxxxx bytes to X:/NEW.UF2" is displayed, writing is complete
