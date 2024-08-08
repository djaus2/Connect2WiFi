#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <SerialBT.h>
#include "Connect2WiFi.h"

// Anonymous (Makes it private to this file.):
namespace{
  //Current Connection details
  String Ssid=DEFAULT_SSID;
  String Passwd=DEFAULT_PASSWORD;
  String Hostname=DEFAULT_HOSTNAME;
  String IoTHubDeviceConnectionString=DEFAULT_DEVICECONNECTIONSTRING;
  String Guid=DEFAULT_GUID;

  bool bSerialDebug = false;
  bool bUseIoTHub = false;
  bool bIncludeGuid = false;

  char eeprom[EEPROM_SIZE] = {0};
}

namespace FlashStorage{

  // Need to set Tools->IP Bluetooth Stack to IPV4 - Bluetooth

  const char * GetDeviceConnectionString()
  {
    return IoTHubDeviceConnectionString.c_str();
  }

  const char * GetDeviceGuidString()
  {
    return Guid.c_str();
  }

  /////////////////////////////////////////////////////////////////////
  // Third Level: Low level read and write EEProm data as strings
  /////////////////////////////////////////////////////////////////////
  bool writeWiFi(String Msg,int start)
  {
    if(Serial) Serial.println("Writing now.");
    EEPROM.begin(EEPROM_SIZE);
    char * msg = const_cast<char*>(Msg.c_str());
    if ((strlen(msg) )> EEPROM_SIZE )
    {
      if(Serial) Serial.println("Message is too big");
      return false;
    }
    int len = strlen(msg);
    for(int i=0; i < len; i++)
    {
      byte val = (byte)msg[i];
      EEPROM.write(start + i, val);
      delay(100); 
    }
    Serial.println("\0");
    EEPROM.write(start + len, 0);
    if (EEPROM.commit()) {
      if(Serial) Serial.println("EEPROM successfully committed");
    } else {
      if(Serial) Serial.println("ERROR! EEPROM commit failed");
    }
    EEPROM.end();
    return true;
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
  /////////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////////
  // Second Level: General purpose read and write of string array data
  /////////////////////////////////////////////////////////////////////
  bool ReadStringArrayFromEEProm(String vals[])
  {
    String datas  = readWiFi(KEYLENGTH);
    
    if(Serial)
    {
      Serial.print("EEProm Data: ");
      Serial.println(datas);
    }
    int length =  datas.length();

    // String should begin and end with SEP_CHAR
    if(datas[0] != SEP_CHAR)
    {
      if(Serial)
        Serial.println("No first -");
      return false;
    }
    if(datas[length-1] != SEP_CHAR)
    {
      if(Serial)
        Serial.println("No last -");
      return false;
    };
    //Nb: There is a leading separator in the string and one after each value.
    int splits[NUM_STORED_VALUES+1];
    int numParts = 1;
    splits[0] = 0;
    for (int i=1;i<NUM_STORED_VALUES;i++)
    {
      int split = datas.indexOf(SEP_CHAR,splits[i-1] +1 );
      if (split == -1)
        break;
      splits[i] = split;
      splits[i+1]= length-1;
      numParts++;
    }
    for (int i=0;i<NUM_STORED_VALUES; i++)
    {
      vals[i] = datas.substring(splits[i]+1,splits[i+1]);
      vals[i].trim();
    }
    if(Serial)
      Serial.println("Done EEProm Read of Data.");
    return true;
  }

  bool WriteStringArray2EEProm(String datas[])
  {
    if (!bSerialDebug) 
      return false;
    if(Serial)
      Serial.println("Writing key to EEProm");
    writeKey();
    String WiFiData = "";
    WiFiData +=  SEP_CHAR;
    for (int i=0;i<NUM_STORED_VALUES;i++)
    {
      WiFiData += datas[i] + SEP_CHAR;
    }
    if(Serial)
    {
      Serial.print("Writing WiFi Config Data to EEProm: ");
      Serial.println(WiFiData);
    }
    String vals[NUM_STORED_VALUES];
    bool res = writeWiFi(WiFiData,KEYLENGTH);
    if(res)
    {
      res = ReadStringArrayFromEEProm(vals);
    }
    if(!res)
    {
      return false;
    }
    for (int i=0;i<NUM_STORED_VALUES;i++)
    {
      if (vals[i] != datas[i])
        return false;
    }
    if(Serial)
      Serial.println("Done EEProm Write of Data.");
    return true;
  }
  /////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Top level: Read/Write as entities
  /////////////////////////////////////////////////////////////////////

  // Read connection details for EEProm
  bool ReadWiFiDataFromEEProm()
  {
    String vals[NUM_STORED_VALUES];
    bool res = ReadStringArrayFromEEProm(vals);
    if(!res)
    {
      return false;
    }

    Ssid = vals[0];
    Passwd = vals[1];
    Hostname = vals[2];

    bUseIoTHub=false;
    bIncludeGuid=false;
    // Next IS IoTHub
    if(vals[3]!= "")
    {
      bUseIoTHub=true;
      IoTHubDeviceConnectionString = vals[3];
    }
    // Next is Guid (order matters)
    // Could test 4 an5 with Guid String Parse. 2Do
    // If no IOT Hub expect a dummy vlue for it.
    if(vals[4]!= "")
    {
      bIncludeGuid=true;
      Guid = vals[4];
    }
    return true;
  }

    // Write data to EEProm but require user notification
  bool Write2EEPromwithPrompt()
  {
    if (!bSerialDebug) 
      return false;
    
    Serial.println("Writing key to EEProm");
    
    writeKey();
    
    String vals[NUM_STORED_VALUES];
    vals[0] = Ssid;
    vals[1] = Passwd;
    vals[2] = Hostname;
    vals[3] = IoTHubDeviceConnectionString;
    vals[4] = Guid;;
    
    bool res = WriteStringArray2EEProm(vals);

    if(!res)
      return false;
    
    // Test EEProm data
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.disconnect();
      delay(200);
    }
    res = ReadWiFiDataFromEEProm();
    if(res)
    {
      res = WiFiConnect();
    }
    return res;
  }

  /////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Keys: Read and write keys (First 4 bytes)
  /////////////////////////////////////////////////////////////////////
  // First 4 bytes of EEProm are the key.
  // Data string follows in the storage.
  // Used to indicate if data has been written.
  // If format changes need to change the key string in the header.
  ///////////////
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
  ////////////////////////////////////////////////////////////////////



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
        if(bIncludeGuid)
        {
          Serial.print("Guid:");
          Serial.println(Guid);
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
    else
    {
      IoTHubDeviceConnectionString="";
    }


    if(bIncludeGuid)
    {
      SerialBT.print("Enter Guid. Default ");
      SerialBT.print(Guid);
      while (SerialBT.available() == 0) {}
      String val = SerialBT.readString();
      val.trim();
      if (val.length()!=0)
        Guid=val;
      Serial.println();
    }
    else
    {
      Guid="";
    }

     return true;
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
      Serial.println(IoTHubDeviceConnectionString);
    }
    else
    {
      IoTHubDeviceConnectionString="";
    }

    if(bIncludeGuid)
    {
      Serial.print("Enter Guid. Default ");
      Serial.print(Guid);
      while (Serial.available() == 0) {}
      String val = Serial.readString();
      val.trim();
      if (val.length()!=0)
        Guid=val;
      Serial.println();
      Serial.println(Guid);
    }
    else
    {
      Guid="";
    }
    Serial.println("Got data");
    return true;
  }

  // Software set connection settings
  void WiFiSet(String ssid, String pwd, String hostname, String deviceconnectionString, String guid )
  {
    Ssid = ssid;
    Passwd = pwd;
    Hostname = hostname;
    IoTHubDeviceConnectionString = deviceconnectionString;
    Guid = guid;
  }

  




  // Orchestrate WiFi Connection
  bool WiFiConnectwithOptions(int baud, ConnectMode connectMode, bool useIoTHub, bool includeGuid, bool debug) 
  {
    bSerialDebug = debug;
    bUseIoTHub = useIoTHub;
    bIncludeGuid = includeGuid;

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

    bool res;
    String resStr;
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
            res = Prompt4WiFiConfigData();
            if(res)
            {
              res = BtWiFiConnect();
            }
            if(!res)
            {
              return false;
            }

            Serial.print("Do you wish to flash the EEProm with this WiFi data? [Y/y] [N/n]");
            while (Serial.available() == 0) {}
            resStr = Serial.readString();
            resStr.toUpperCase();
            resStr.trim();
            Serial.println();
            if (resStr =="Y")
            {
              res =  Write2EEPromwithPrompt();
              if(res)
              {
              res = ReadWiFiDataFromEEProm();
              }
              if(res)
              {
                res = BtWiFiConnect();
              }
              return res;
            }
            else
            {
              return false;
            }
          }
        }
        else
        {
          String resStr="N";
          bool res = false;
          if (bSerialDebug)
          {
            Serial.print("Key found. (");
            Serial.print(KEY);
            Serial.println(") at start of EEProm"); 
            Serial.print("Do you wish to reflash the EEProm with new WiFi data? [Y/y] [N/n]");
            while (Serial.available() == 0) {}
            resStr = Serial.readString();
            resStr.toUpperCase();
            resStr.trim();
          }
          Serial.println();
          if (resStr =="Y")
          {
            res = Prompt4WiFiConfigData();
            if(res)
            {
              //Check
              res= WiFiConnect();
            }
            if(res)
            {
              //Write if check OK
              res =  Write2EEPromwithPrompt();
            }
            if(! res)
              return false;
          }
          res = ReadWiFiDataFromEEProm();
          if(res)
          {
            res= WiFiConnect();
          }
          return res;
        }
        break;
      case is_defined:
        WiFiSet(DEFAULT_SSID, DEFAULT_PASSWORD, DEFAULT_HOSTNAME, DEFAULT_DEVICECONNECTIONSTRING, DEFAULT_GUID);    
        return WiFiConnect();
        break;
      case serial_prompt:
        res = Prompt4WiFiConfigData();
        if(res)
        {
          res= WiFiConnect();
        }
        return res;
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
        if(resBt)
        {
          return WiFiConnect();
        }
        return resBt;
        break;
      
    }
    return false;
  }

  /////////////////////////////////////////////////////////////








  



}