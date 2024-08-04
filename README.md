# Connect2WiFi - Arduino

Various options for connect to a RPi Pico W with Arduino BSP to WiFi.

----

## Update

- Azure IoT Hub Device Connection String added.
  - Can flash it and read from flash etc.

---

## Options

- Connect from EEProm (simulated) settings
  - Option to write new set if setting found/not found there*.
- Connect with embedded settings in Header
- Prompt for settings over Serial
- Prompt for settings over Bluetooth
- Call with settings as parameters
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
  - I use a Bluetooth terminal on an Android Phone:
    - [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en_US)  _from GooglePlay._
- Suggest using this sketch to set EEProm

## Using in another Sketch

- Add the 2 files
- #include the the header file in target sketch source file where used.
- Call WiFiConnectwithOptions(int baud, ConnectMode connectMode, bool debug)
  - ConnectMode:
    - wifi_is_set
      - Call WiFiSet(String ssid, String pwd, String hostname) first  
    - from_eeprom
    - is_defined
      - Use values defined in header
    - serial_prompt
    -   bt_prompt
- _If Serial is used but not started  when WiFiConnectwithOptions() is called, then baud is used in starting it._
