#include <SoftwareSerial.h>

SoftwareSerial lora(13, 15);

String packet;

unsigned long lastReceive;

void setup() {
  lora.begin(19200);
  Serial.begin(115200);
  Serial.println("Base station");
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

  if (millis() - lastReceive >= 2000) packet = "";
}

void processPacket(String packet) {
  Serial.println("Packet received: " + packet);
  byte firstSeparator = packet.indexOf((char)',');
  byte secondSeparator = packet.indexOf((char)',', firstSeparator + 1);
  byte thirdSeparator = packet.indexOf((char)',', secondSeparator + 1);

  int destinationId = packet.substring(0, firstSeparator).toInt();
  int originId = packet.substring(firstSeparator + 1, secondSeparator).toInt();
  char command = packet.charAt(secondSeparator + 1);
  String message = packet.substring(thirdSeparator + 1);

  if (destinationId == 100) {
    Serial.println("Replying to node...");
    replyToNode(originId, destinationId, command, "OK");
  }
}


// Reply to node.
void replyToNode(int destinationId, int originId, char command, String message) {
  String payload =
    String(destinationId) + "," +
    String(originId) + "," +
    command + "," +
    message;

  //uint32_t checksum = calculateCRC32(payload.toCharArray(), payload.length());

  // Send packet.
  lora.print(payload);
  lora.write(0x04);
}
