#include <Arduino.h>
#include "Connect2WiFi.h"

// In test sketch:
#define ENABLE_SERIAL_AT_START true

// Test Sketch. Also use to set EEProm with WiFi data

bool test=false;
int del = 2500;

void setup()
{
  bool _serialDebug = false;
  if(ENABLE_SERIAL_AT_START)
  {
    Serial.begin();
    while (!Serial);
  }

  if(Serial)
  {
    String res ="Y";
    Serial.print("Do you wish to display Debug messages in Serial Window?");
    while (Serial.available() == 0) {}
    res = Serial.readString();
    res.toUpperCase();
    res.trim();
    Serial.println();
    if (res =="Y")
    {
      _serialDebug = true;
      Serial.println("Started");
    }
  }
  
  // Display flash signature: Started
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(1000);                    
  digitalWrite(LED_BUILTIN, LOW); 
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(250);                    
  digitalWrite(LED_BUILTIN, LOW); 
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH); 
    delay(250);                    
  digitalWrite(LED_BUILTIN, LOW); 
  delay(250);
    digitalWrite(LED_BUILTIN, HIGH); 
  delay(1000);                    
  digitalWrite(LED_BUILTIN, LOW); 
  delay(1000);

  // enum ConnectMode: byte {wifi_is_set, from_eeprom, is_defined, serial_prompt, bt_prompt };
  ConnectMode _connectMode = from_eeprom;

  test = FlashStorage::WiFiConnectwithOptions(115200,_connectMode,true, _serialDebug);
  pinMode(LED_BUILTIN, OUTPUT);
  if(test)
    del = 250;
}

void loop()
{
  // Short flashes means success
  // Long flashes means failure
  // So can see result without Serial
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(del);                    
  digitalWrite(LED_BUILTIN, LOW); 
  delay(del);  
}




