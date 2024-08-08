#ifndef CONNECT2WiFi
#define CONNECT2WiFi

#define DEFAULT_SSID "APQLZM"
#define DEFAULT_PASSWORD "tallinn187isnotinaustralia"
#define DEFAULT_HOSTNAME "picow"
#define DEFAULT_BT_NAME "picow SoftataOTA 00:00:00:00:00:00"
#define DEFAULT_DEVICECONNECTIONSTRING "HostName=picow.azure-devices.net;DeviceId=picow;SharedAccessKey=0"
#define DEFAULT_GUID "00000000-0000-0000-0000-000000000000"
#define EEPROM_SIZE 256

// Using the 
#define SEP_CHAR '|'
#define NUM_STORED_VALUES 5
#define MIN_NUM_STORED_VALUES 3

// Key is used to see if the WiFi data has been written to the EEProm
#define KEY  "1370"
#define KEYLENGTH 4

enum ConnectMode: byte {wifi_is_set, from_eeprom, is_defined, wifiset, serial_prompt, bt_prompt };


namespace FlashStorage{

static const char *ConnectMode_STR[] = {
    "wifi_is_set", "from_eeprom", "is_defined", "wifiset", "serial_prompt", "bt_prompt" 
};

const char * GetDeviceConnectionString();

// Read key (in first 4 bytes ) and check
bool readKey();
// Write key in first 4 bytes
void writeKey();

//////////////////////////////////////
// WiFi Data is stored after the key as string: SSID-Passsword-Hostname
void writeWiFi(String Msg,int start);
String readWiFi(int start);

//////////////////////////////////////

bool ReadStringArrayFromEEProm( String datas[]);
bool WriteStringArray2EEProm(String datas[]);

//////////////////////////////////////

bool ReadWiFiDataFromEEProm();
bool Write2EEPromwithPrompt();

//////////////////////////////////////


// WiFi connect with current settings
bool WiFiConnect();

// Prompt user for WiFi connection details and connect
bool Prompt4WiFiConfigData();

// Software set connection settings and connect
void WiFiSet(String ssid, String pwd, String hostname, String deviceconnectionString, String guid );

// Orchestrate WiFi Connection
bool WiFiConnectwithOptions(int baud, ConnectMode connectMode, bool iothub, bool guid, bool debug);
}

#endif
