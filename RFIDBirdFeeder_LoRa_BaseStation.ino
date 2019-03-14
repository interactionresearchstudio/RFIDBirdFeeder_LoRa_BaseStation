// Libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <SoftwareSerial.h>

// DEBUG - uncomment for debug info via serial
#define DEBUG

#define RADIOID 100

// Debug print macros
#ifdef DEBUG
#define DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define DEBUG_PRINTLN(x)
#endif
#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print(x)
#else
#define DEBUG_PRINT(x)
#endif
#ifdef DEBUG
#define DEBUG_PRINTHEX(x)  Serial.print(x, 16)
#else
#define DEBUG_PRINTHEX(x)
#endif

// CONFIG DEFINES
char WLAN_SSID[32];
char WLAN_PASS[32];
#define HOST "http://feedernet.herokuapp.com"
String FEEDERSTUB = " ";
#define REQUEST_RETRIES 2
#define WIFI_REGULAR_MAX_RETRIES 600
#define PACKET_TIMEOUT 1000

SoftwareSerial lora(13, 15);

String packet;

unsigned long lastReceive;

void setup() {
  lora.begin(19200);
  Serial.begin(115200);
  Serial.println("Base station");

  FEEDERSTUB = WiFi.macAddress();
  WiFi.mode(WIFI_OFF);
  digitalWrite(14, HIGH);
  DEBUG_PRINTLN(" ");
  DEBUG_PRINT("Base Station Firmware");
  DEBUG_PRINT(" | MAC: ");
  DEBUG_PRINTLN(FEEDERSTUB);
  DEBUG_PRINT("Reset reason: ");
  DEBUG_PRINTLN(ESP.getResetReason());
  DEBUG_PRINTLN("Reset from Powerup. Press W to change WiFi credentials...");
  delay(1500);
  updateUart();
  connectToWiFi();
}

void loop() {
  if (lora.available() > 0) {
    char inChar = lora.read();
    Serial.print(inChar);
    if (inChar == 0x04) {
      processPacket(packet);
      packet = "";
    }
    else packet += inChar;
    lastReceive = millis();
  }

  // Wipe packet buffer on timeout.
  if (millis() - lastReceive >= PACKET_TIMEOUT) packet = "";
}

// Process packet from node
void processPacket(String packet) {
  Serial.println("Packet received: " + packet);
  byte firstSeparator = packet.indexOf((char)',');
  byte secondSeparator = packet.indexOf((char)',', firstSeparator + 1);
  byte thirdSeparator = packet.indexOf((char)',', secondSeparator + 1);

  int destinationId = packet.substring(0, firstSeparator).toInt();
  int originId = packet.substring(firstSeparator + 1, secondSeparator).toInt();
  char command = packet.charAt(secondSeparator + 1);
  String message = packet.substring(thirdSeparator + 1);

  FEEDERSTUB = WiFi.macAddress() + ":" + String(originId);

  if (destinationId == 100) {
    // Packet is destined for the base station.
    Serial.println("Executing node request...");
    if (command == 'R') {
      postTrack(message);
      replyToNode(originId, destinationId, command, "OK");
    }
    else if (command == 'T') {
      uint32_t newTime = getTime();
      replyToNode(originId, destinationId, command, String(newTime));
    }
    else if (command == 'P') {
      sendPing();
      replyToNode(originId, destinationId, command, "OK");
    }
    else if (command == 'U') {
      sendPowerup();
      replyToNode(originId, destinationId, command, "OK");
    }
  }
}

// Reply to node.
void replyToNode(int destinationId, int originId, char command, String message) {
  String payload =
    String(destinationId) + "," +
    String(originId) + "," +
    command + "," +
    message;

  // Send packet.
  lora.print(payload);
  lora.write(0x04);
}
