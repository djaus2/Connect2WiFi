#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <SerialBT.h>
#include "Connect2WiFi.h"

// Anonymous (Makes it privateto this file.):
namespace{
  //Current Connection details
  String Ssid="DEFAULT_SSID";
  String Passwd="DEFAULT_PASSWORD";
  String Hostname=DEFAULT_HOSTNAME;
  String IoTHubDeviceConnectionString=DEFAULT_DEVICECONNECTIONSTRING;

  bool bSerialDebug = false;
  bool bUseIoTHub = false;

  char eeprom[EEPROM_SIZE] = {0};
}

namespace FlashStorage{

  // Need to set Tools->IP Bluetooth Stack to IPV4 - Bluetooth

  const char * GetDeviceConnectionString()
  {
    return IoTHubDeviceConnectionString.c_str();
  }

  // WiFi connect with current settings
  bool WiFiConnect()
  {
      if(bSerialDebug)
      {
        Serial.print("SSID:");
        Serial.println(Ssid);
        Serial.print("Password:");
        Serial.println(Passwd);
        Serial.print("Hostname:");
        Serial.println(Hostname);
        if(bUseIoTHub)
        {
          Serial.print("DeviceConnectionString:");
          Serial.println(IoTHubDeviceConnectionString);
        }

        Serial.println("Connecting to WiFi");
      }

      WiFi.mode(WIFI_STA);
      WiFi.setHostname(Hostname.c_str());
      WiFi.begin(Ssid.c_str(), Passwd.c_str());

      //2Do add timeout here
      while (WiFi.status() != WL_CONNECTED) {
        if(bSerialDebug)
        {
          Serial.print(".");
        }
        delay(250);
      }
      if(bSerialDebug)
      {
        Serial.println();
        Serial.println("Connected.");
      }
      
      //print the local IP address
      IPAddress ip = WiFi.localIP();
      if(bSerialDebug)
      {
        Serial.print("RPi Pico W IP Address: ");
        Serial.println(ip);
      }
      return true;
  }

  // As per WiFiConnect but use Bluetooth Serial instead of Serial
  bool BtWiFiConnect()
  {
      if(true)
      {
        SerialBT.print("SSID:");
        SerialBT.println(Ssid);
        SerialBT.print("Password:");
        SerialBT.println(Passwd);
        SerialBT.print("Hostname:");
        SerialBT.println(Hostname);

        SerialBT.println("Connecting to WiFi");
      }

      WiFi.mode(WIFI_STA);
      WiFi.setHostname(Hostname.c_str());
      WiFi.begin(Ssid.c_str(), Passwd.c_str());

      //2Do add timeout here
      while (WiFi.status() != WL_CONNECTED) {
        SerialBT.print(".");
        delay(250);
      }
      SerialBT.println();
      SerialBT.println("Connected.");
      
      //print the local IP address
      IPAddress ip = WiFi.localIP();
      SerialBT.print("RPi Pico W IP Address: ");
      SerialBT.println(ip);
      return true;
  }

  // Read connection details for EEProm
  bool ReadWiFiDataFromEEProm()
  {
    String data  = readWiFi(KEYLENGTH);
    data.trim();
    int split = data.indexOf('-');
    int split2 = data.indexOf('-',split+1);



    Ssid = data.substring(0,split);
    Ssid.trim();
    Passwd = data.substring(split+1,split2);
    Passwd.trim();
    Hostname = data.substring(split2+1,data.length());
    Hostname.trim();

    if(bUseIoTHub)
    {
      int split3 = data.indexOf('-',split2+1);
      Hostname = data.substring(split2+1,split3);
      Hostname.trim();
      IoTHubDeviceConnectionString = data.substring(split3+1,data.length());
      IoTHubDeviceConnectionString.trim();
    }

    return WiFiConnect();
  }

  // Prompt user for WiFi connection details
  // Assumes BT is connected
  bool BTPrompt4WiFiConfigData()
  {
    //Get SSID     
    SerialBT.print("Enter SSID: ");
    while (SerialBT.available() == 0) {}
    Ssid = SerialBT.readString();
    Ssid.trim();
    Serial.println();

    //Get Password
    SerialBT.print("Enter Password: ");
    while (SerialBT.available() == 0) {}
    Passwd = SerialBT.readString();
    Passwd.trim();
    Serial.println();

    //Get Hostname
    SerialBT.print("Enter Hostname. Default ");
    SerialBT.print(Hostname);
    while (SerialBT.available() == 0) {}
    String val = SerialBT.readString();
    val.trim();
    if (val.length()!=0)
      Hostname=val;
    Serial.println();

    if(bUseIoTHub)
    {
      SerialBT.print("Enter IoT Hub Connection String. Default ");
      SerialBT.print(IoTHubDeviceConnectionString);
      while (SerialBT.available() == 0) {}
      String val = SerialBT.readString();
      val.trim();
      if (val.length()!=0)
        IoTHubDeviceConnectionString=val;
      Serial.println();
    }

    return BtWiFiConnect();
  }

  // Prompt user for WiFi connection details
  // Requires to be in Debug mode
  bool Prompt4WiFiConfigData()
  {
    if(!bSerialDebug)
      return false;

    //Get SSID     
    Serial.print("Enter SSID: ");
    while (Serial.available() == 0) {}
    Ssid = Serial.readString();
    Ssid.trim();
    Serial.println();

    //Get Password
    Serial.print("Enter Password: ");
    while (Serial.available() == 0) {}
    Passwd = Serial.readString();
    Passwd.trim();
    Serial.println();

    //Get Hostname
    Serial.print("Enter Hostname. Default ");
    Serial.print(Hostname);
    while (Serial.available() == 0) {}
    String val = Serial.readString();
    val.trim();
    if (val.length()!=0)
      Hostname=val;
    Serial.println();
    
    if(bUseIoTHub)
    {
      Serial.print("Enter IoT Hub Connection String. Default ");
      Serial.print(IoTHubDeviceConnectionString);
      while (Serial.available() == 0) {}
      String val = Serial.readString();
      val.trim();
      if (val.length()!=0)
        IoTHubDeviceConnectionString=val;
      Serial.println();
    }

    return BtWiFiConnect();
  }

  // Software set connection settings
  void WiFiSet(String ssid, String pwd, String hostname, String deviceconnectionString )
  {
    Ssid = ssid;
    Passwd = pwd;
    Hostname = hostname;
    IoTHubDeviceConnectionString = deviceconnectionString;
  }

  // Write data to EEProm but require user notification
  bool Write2EEPromwithPrompt()
  {
    if (!bSerialDebug) 
      return false;
    Serial.println("Writing key to EEProm");
    writeKey();
    String WiFiData = Ssid +'-'+ Passwd + "-" + Hostname;
    if(bUseIoTHub)
    {
      WiFiData += "-" + IoTHubDeviceConnectionString;
    }
    Serial.print("Writing WiFi Config Data to EEProm: ");
    Serial.println(WiFiData);

    writeWiFi(WiFiData,KEYLENGTH);

    // Test EEProm data
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.disconnect();
      delay(200);
    }
    ReadWiFiDataFromEEProm();
    return WiFiConnect();
  }

  // Orchestrate WiFi Connection
  bool WiFiConnectwithOptions(int baud, ConnectMode connectMode, bool useIoTHub, bool debug) 
  {
    bSerialDebug = debug;
    bUseIoTHub = useIoTHub;

    if(bSerialDebug)
    {
      if(!Serial)
      {
        Serial.begin(baud);
      }
      while(!Serial);
      delay(100);
      Serial.print("WiFi ConnectMode: ");
      Serial.println(ConnectMode_STR[connectMode]);
    }


    switch (connectMode)
    {
      case wifi_is_set:
        return WiFiConnect();
        break;
      case from_eeprom:
        if(!readKey())
        {
          if(!bSerialDebug)
          {
            return false;
          }
          else
          {
            Serial.print("Key not found (");
            Serial.print(KEY);
            Serial.println(") at start of EEProm"); 

            Serial.println("Getting WiFi config data.");
            Prompt4WiFiConfigData();
            Serial.print("Do you wish to flash the EEProm with this WiFi data? [Y/y] [N/n]");
            while (Serial.available() == 0) {}
            String res = Serial.readString();
            res.toUpperCase();
            res.trim();
            Serial.println();
            if (res =="Y")
            {
              return Write2EEPromwithPrompt();
            }
            else
            {
              return false;
            }
          }
        }
        else
        {
          String res="N";
          if (bSerialDebug)
          {
            Serial.print("Key found. (");
            Serial.print(KEY);
            Serial.println(") at start of EEProm"); 
            Serial.print("Do you wish to reflash the EEProm with new WiFi data? [Y/y] [N/n]");
            while (Serial.available() == 0) {}
            res = Serial.readString();
            res.toUpperCase();
            res.trim();
          }
          Serial.println();
          if (res =="Y")
          {
            Prompt4WiFiConfigData();
            return Write2EEPromwithPrompt();
          }
          else
          {
            ReadWiFiDataFromEEProm();
            return WiFiConnect();
          }
        }
        break;
      case is_defined:
        WiFiSet(DEFAULT_SSID, DEFAULT_PASSWORD, DEFAULT_HOSTNAME, DEFAULT_DEVICECONNECTIONSTRING);    
        return WiFiConnect();
        break;
      case serial_prompt:
        return Prompt4WiFiConfigData();
        break;
      case bt_prompt:
        SerialBT.setName(DEFAULT_BT_NAME);
        SerialBT.begin();
        while (!SerialBT);  
        if(Serial) 
        {
          Serial.println("BT Started.");
          Serial.println("Connect BT Terminal now.");
          Serial.println("Once connected, in BT Terminal press |> to continue.");
        }
        while (!SerialBT.available());
        delay(100);
        String dummy = SerialBT.readString();
        bool resBt =  BTPrompt4WiFiConfigData();
        return resBt;
        break;
      
    }
    return false;
  }

  /////////////////////////////////////////////////////////////






  void writeWiFi(String Msg,int start)
  {
    EEPROM.begin(EEPROM_SIZE);
    char * msg = const_cast<char*>(Msg.c_str());
    if ((strlen(msg) )> EEPROM_SIZE )
    {
      Serial.println("Message is too big");
      return;
    }
    int len = strlen(msg);
    for(int i=0; i < len; i++)
    {
      byte val = (byte)msg[i];
      EEPROM.write(start + i, val);
      delay(100); 
    }
    EEPROM.write(start + len, 0);
    if (EEPROM.commit()) {
      Serial.println("EEPROM successfully committed");
    } else {
      Serial.println("ERROR! EEPROM commit failed");
    }
    EEPROM.end();
  }



  String readWiFi(int start)
  {
    EEPROM.begin(EEPROM_SIZE);
    int address = start;
    byte value = 0;
    String target ="";
    while (((value = EEPROM.read(address++))!= 0) && (address < EEPROM_SIZE))
    {
      char ch = (char)value;
      target += ch;
      delay(100);
    }
    target.trim();
    EEPROM.end();
    return target;
  }

  // First 4 bytes of EEProm are the key then just read WiFi data.
  // Else write key and data.
  void writeKey()
  {
    String key = KEY;
    EEPROM.begin(EEPROM_SIZE);
    char * msg = const_cast<char*>(key.c_str());
    int len = KEYLENGTH;
    for(int i=0; i < len; i++)
    {
      byte val = (byte)msg[i];
      EEPROM.write(i, val);
      delay(100); 
    }
    EEPROM.write(len, 0);
    if (EEPROM.commit()) {
      Serial.println("EEPROM key write successfully committed");
    } else {
      Serial.println("ERROR! EEPROM key write commit failed");
    }
    EEPROM.end();
  }

  bool readKey()
  {
    String key = KEY;
    EEPROM.begin(EEPROM_SIZE);
    int address = 0;
    byte value = 0;
    String target ="";
    for (int i=0;i< KEYLENGTH;i++)
    {
      byte value  = EEPROM.read(i);
      char ch = (char)value;
      target += ch;
      delay(100);
    }
    target.trim();
    EEPROM.end();
    if (target==key)
      return true;
    else
      return false;
  }



}