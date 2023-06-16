#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()

#define CHANNEL 11

// Global copy of slave
esp_now_peer_info_t slave;

const int PIN_SENSOR = 21;


void setup() {
  pinMode(PIN_SENSOR, INPUT);
  Serial.begin(115200);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  InitESPNow();
  esp_now_register_send_cb(OnDataSent);
  ScanForSlave();
  esp_now_add_peer(&slave);
}


void loop() {
    
  uint8_t motionValue = digitalRead(PIN_SENSOR);
  
  Serial.println(motionValue);
  
  // In the loop we scan for the slave
  ScanForSlave();
  // If the slave is found, it will be populated in the `slave` variable

  esp_now_send(slave.peer_addr, reinterpret_cast<const uint8_t*>(&motionValue), sizeof(motionValue));
  
  delay(3000);
}


// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// Scan for slaves in AP mode 
void ScanForSlave() {
  int16_t scanResults = WiFi.scanNetworks(); // Scan only on one channel
  
  for (int i = 0; i < scanResults; ++i) {
    // Print SSID and RSSI for each device found
    String SSID = WiFi.SSID(i);
    int32_t RSSI = WiFi.RSSI(i);
    String BSSIDstr = WiFi.BSSIDstr(i);

      
    // Check if the current device starts with `Slave`
    if (SSID.indexOf("Slave") == 0) {
       
      int mac[6];
      if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
        for (int ii = 0; ii < 6; ++ii ) {
          slave.peer_addr[ii] = (uint8_t) mac[ii];
        }
      }

      slave.channel = CHANNEL; // pick a channel
      slave.encrypt = 0; // no encryption
      break;
    }
  }
}


// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
