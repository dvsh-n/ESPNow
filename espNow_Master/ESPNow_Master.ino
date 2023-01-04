#include <esp_now.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()
#include <WiFi.h>

#define CHANNEL 1

void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK)
    Serial.println("ESPNow Init Success");
  else {
    Serial.println("ESPNow Init Fail, restarting ESP Device");
    ESP.restart();
  }
}

esp_now_peer_info_t genSlave(const String mac_addr){
  esp_now_peer_info_t slave;
  int mac[6];
  sscanf(mac_addr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
  for(int i = 0; i < 6; i++)
    slave.peer_addr[i] = (uint8_t) mac[i];
  slave.channel = CHANNEL;
  slave.encrypt = 0; 
  return slave;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void addPeer(esp_now_peer_info_t slave) {
  String Status;
  bool exists  = esp_now_is_peer_exist(slave.peer_addr);
  if (exists) 
    Status  = "Already Paired";
  else {
    esp_err_t result = esp_now_add_peer(&slave);
    switch(result){
      case ESP_OK:
        Status = "Peer Added";
      case ESP_ERR_ESPNOW_NOT_INIT:
        Status = "ESPNOW not Init";
      case ESP_ERR_ESPNOW_ARG:
        Status = "Invalid Argument";
      case ESP_ERR_ESPNOW_FULL:
        Status = "Peer list full";
      case ESP_ERR_ESPNOW_NO_MEM:
        Status = "Out of memory";
      case ESP_ERR_ESPNOW_EXIST:
        Status = "Peer exists";
      default:
        Status = "Not sure what happened";
    }
  }
  Serial.println(Status);
}

void sendData(uint8_t data, esp_now_peer_info_t slave) {
  String Status;
  const uint8_t *peer_addr = slave.peer_addr;
  esp_err_t result = esp_now_send(peer_addr, &data, sizeof(data));
  switch(result) {
    case ESP_OK:
      Status = "Success";
    case ESP_ERR_ESPNOW_NOT_INIT:
      Status = "ESPNOW not Init";
    case ESP_ERR_ESPNOW_ARG:
      Status = "Invalid Argument";
    case ESP_ERR_ESPNOW_INTERNAL:
      Status = "Internal Error";
    case ESP_ERR_ESPNOW_NO_MEM:
      Status = "Out of memory";
    case ESP_ERR_ESPNOW_NOT_FOUND:
      Status = "Peer not found";
    default:
      Status = "Not sure what happened";
  }
  Serial.println(Status);
}

void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  InitESPNow();
  //Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  //Serial.print("STA CHANNEL "); Serial.println(WiFi.channel());
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  esp_now_peer_info_t slave1 = genSlave("E0:5A:1B:75:A7:B5");
  addPeer(slave1);
  sendData(0x20, slave1);
  delay(1000);
}
