// Connect to WiFi - attempt at quick connect, fall back to normal connect.
void connectToWiFi() {
  loadCredentials();
  WiFi.mode(WIFI_STA);
  DEBUG_PRINTLN("Performing regular connect...");
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    retries++;
    DEBUG_PRINT("Attempt ");
    DEBUG_PRINTLN(retries);

    if (retries == WIFI_REGULAR_MAX_RETRIES) {
      DEBUG_PRINTLN("Giving up on WiFi...");
      WiFi.disconnect(true);
      WiFi.forceSleepBegin();
      delay(1);
      WiFi.mode(WIFI_OFF);
      return;
    }
    delay(50);
  }
  DEBUG_PRINTLN("Connected to WiFi.");
}

// Read WiFi credentials from UART.
void readCredentialsFromUart() {
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  Serial.flush();
  Serial.print("Please insert new SSID: ");
  String ssid, password;
  boolean foundReturn = false;
  while (!foundReturn) {
    if (Serial.available() > 0) {
      char inChar = Serial.read();
      if (inChar == '\r') {
        foundReturn = true;
        Serial.println();
      }
      else {
        ssid += inChar;
        Serial.print(inChar);
      }
    }
  }
  Serial.print("Please insert new password: ");
  foundReturn = false;
  while (!foundReturn) {
    if (Serial.available() > 0) {
      char inChar = Serial.read();
      if (inChar == '\r') {
        foundReturn = true;
        Serial.println();
      }
      else {
        password += inChar;
        Serial.print(inChar);
      }
    }
  }
  ssid.toCharArray(WLAN_SSID, 32);
  password.toCharArray(WLAN_PASS, 32);
  saveCredentials();
}

// UART functions
void updateUart() {
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    if (inChar == 'W') {
      readCredentialsFromUart();
    }
  }
}

// Save WiFi credentials to EEPROM.
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, WLAN_SSID);
  EEPROM.put(0 + sizeof(WLAN_SSID), WLAN_PASS);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(WLAN_SSID) + sizeof(WLAN_PASS), ok);
  EEPROM.commit();
  EEPROM.end();
  DEBUG_PRINTLN("Saved WiFi credentials to EEPROM.");
}

// Load WiFi credentials from EEPROM
void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, WLAN_SSID);
  EEPROM.get(0 + sizeof(WLAN_SSID), WLAN_PASS);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(WLAN_SSID) + sizeof(WLAN_PASS), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    WLAN_SSID[0] = 0;
    WLAN_PASS[0] = 0;
    DEBUG_PRINTLN("Wireless credentials not set. Please set now.");
    readCredentialsFromUart();
  }
  DEBUG_PRINT("Recovered credentials: ");
  DEBUG_PRINT(WLAN_SSID);
  DEBUG_PRINTLN(strlen(WLAN_PASS) > 0 ? " | ********" : " | <no password>");
}
