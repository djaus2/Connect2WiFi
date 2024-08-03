# Connect2WiFi - Arduino

Various options for connect to a RPi Pico W with Arduino BSP to WiFi.

## Options

- Connect from EEProm (simulated) settings
  - Option to write new set if setting found/not found there*.
- Connect with embedded settings in Header
- Prompt for settings over Serial
- Prompt for settings over Bluetooth
- _(Further)_ Option to use Serial debug nessages (which is blocking) or not.

#### * EEProm Format

- 256 bytes in flash reserved for this
- First 4 bytes are a key, _(see header)_, used to detect if settings have been written.
- Format after key is &lt;SSID&gt;-&lt;Password&gt;-&lt;Hostname&gt;

## Provisioning a RPi Pico W with Arduino

- BSP: See [github/earlephilhower/arduino-pico](https://github.com/earlephilhower/arduino-pico)
- RPi Pico W Simulated EEProm Docs: See [EEPROM Library](https://arduino-pico.readthedocs.io/en/latest/eeprom.html)

## Notes

- By placing the settings in the simulated EEProm, the device can rrun "headless".
- Whilst the sketch will run as a test, the source files can be used as a library in another sketch.
- If Using Bluetooth option, start device, pair from a Bluetooth Terminal, send a blank message to start.
  - Will be prompted in Bluetooth terminal for settings.
  - I use an Bluetooth terminal on Android Phone:
    - [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en_US)  _from GooglePlay._
