// Basic GET request.
String getRequest(char* endpoint, int *httpCode, byte maxRetries) {
  HTTPClient http;

  for (int i = 0; i < maxRetries; i++) {
    http.begin(String(HOST) + String(endpoint));

    *httpCode = http.GET();

    String payload;
    if (*httpCode == HTTP_CODE_OK || *httpCode == 400) {
      DEBUG_PRINT("HTTP code: ");
      DEBUG_PRINTLN(*httpCode);
      payload = http.getString();
      DEBUG_PRINTLN(payload);
      return payload;
    }
    else {
      DEBUG_PRINT("HTTP GET failed. Code: ");
      DEBUG_PRINTLN(*httpCode);
      if (maxRetries > 0) {
        DEBUG_PRINT("Retry #");
        DEBUG_PRINTLN(i + 1);
      }
    }
  }

  return String("");
}

// GET request with payload.
String getRequest(char* endpoint, String request, int *httpCode, byte maxRetries) {
  HTTPClient http;

  for (int i = 0; i < maxRetries; i++) {
    http.begin(String(HOST) + String(endpoint));
    http.addHeader("Content-Type", "application/json");

    *httpCode = http.sendRequest("GET", request);

    String result;
    if (*httpCode == 200) {
      DEBUG_PRINT("HTTP code: ");
      DEBUG_PRINTLN(*httpCode);
      result = http.getString();
      DEBUG_PRINTLN(result);
      return result;
    }
    else {
      DEBUG_PRINT("HTTP GET failed. Code: ");
      DEBUG_PRINTLN(*httpCode);
      if (maxRetries > 0) {
        DEBUG_PRINT("Retry #");
        DEBUG_PRINTLN(i + 1);
      }
    }
  }

  return String("");
}

// Basic POST request.
String postRequest(char* endpoint, String request, int *httpCode, byte maxRetries) {
  HTTPClient http;

  for (int i = 0; i < maxRetries; i++) {
    http.begin(String(HOST) + String(endpoint));
    http.addHeader("Content-Type", "application/json");

    *httpCode = http.POST(request);

    String result;
    if (*httpCode == 200) {
      DEBUG_PRINT("HTTP code: ");
      DEBUG_PRINTLN(*httpCode);
      result = http.getString();
      DEBUG_PRINTLN(result);
      return result;
    }
    else {
      DEBUG_PRINT("HTTP POST failed. Code: ");
      DEBUG_PRINTLN(*httpCode);
      if (maxRetries > 0) {
        DEBUG_PRINT("Retry #");
        DEBUG_PRINTLN(i + 1);
      }
    }
  }

  return String("");
}

// Get Unix time from server.
unsigned long getTime() {
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  int httpCode;
  String timeJson = getRequest("/api/time", &httpCode, REQUEST_RETRIES);
  if (httpCode != 200) {
    DEBUG_PRINTLN("Could not retrieve time from server.");
    return 0;
  }
  JsonObject& root = jsonBuffer.parseObject(timeJson);
  if (!root.success()) {
    DEBUG_PRINTLN("Could not parse JSON object.");
    return 0;
  }
  return root["time"];
}

// Send tracking event to server.
void postTrack(String rfid) {
#ifdef LORA
  requestFromRadio(100, RADIOID, 'R', rfid, 4000, 3);
  //if (reply != "") DEBUG_PRINTLN("Radio request failed.");
#else
  const size_t bufferSize = JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();
  root["datetime"] = " ";
  root["rfid"] = rfid;
  root["stub"] = FEEDERSTUB;

  String payload;
  root.printTo(payload);
  DEBUG_PRINT("Payload: ");
  DEBUG_PRINTLN(payload);
  DEBUG_PRINTLN("Posting track...");
  int httpCode;
  String res = postRequest("/api/recordTrack", payload, &httpCode, REQUEST_RETRIES);
  if (httpCode != 200) {
    DEBUG_PRINTLN("Post request failed. Caching track...");
  }
#endif
}

// Send ping to server
void sendPing() {
  const size_t bufferSize = JSON_OBJECT_SIZE(1);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();
  root["stub"] = FEEDERSTUB;

  String payload;
  root.printTo(payload);
  DEBUG_PRINT("Payload: ");
  DEBUG_PRINTLN(payload);
  DEBUG_PRINTLN("Sending Ping...");
  int httpCode;
  String res = postRequest("/api/ping", payload, &httpCode, REQUEST_RETRIES);
  DEBUG_PRINTLN("Result: ");
  DEBUG_PRINTLN(res);
}

// Send powerup event to server
void sendPowerup() {
  const size_t bufferSize = JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();
  root["stub"] = FEEDERSTUB;
  root["type"] = "powerup";

  String payload;
  root.printTo(payload);
  DEBUG_PRINT("Payload: ");
  DEBUG_PRINTLN(payload);
  DEBUG_PRINTLN("Sending Powerup Event...");
  int httpCode;
  String res = postRequest("/api/ping", payload, &httpCode, REQUEST_RETRIES);
  DEBUG_PRINTLN("Result: ");
  DEBUG_PRINTLN(res);
}

// Send low battery event to server
void sendLowBattery() {
  const size_t bufferSize = JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();
  root["stub"] = FEEDERSTUB;
  root["type"] = "lowbattery";

  String payload;
  root.printTo(payload);
  DEBUG_PRINT("Payload: ");
  DEBUG_PRINTLN(payload);
  DEBUG_PRINTLN("Sending Low Battery Event...");
  int httpCode;
  String res = postRequest("/api/ping", payload, &httpCode, REQUEST_RETRIES);
  DEBUG_PRINTLN("Result: ");
  DEBUG_PRINTLN(res);
}

String getSunriseSunset() {
  const size_t bufferSize = JSON_OBJECT_SIZE(1);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();
  root["stub"] = FEEDERSTUB;

  String payload;
  root.printTo(payload);
  DEBUG_PRINT("Payload: ");
  DEBUG_PRINTLN(payload);
  DEBUG_PRINTLN("Getting sunrise / sunset...");
  int httpCode;
  String res = getRequest("/api/time/sunrisesunset", payload, &httpCode, REQUEST_RETRIES);
  DEBUG_PRINTLN("Result: ");
  DEBUG_PRINTLN(res);

  const size_t capacity = 3 * JSON_OBJECT_SIZE(2) + 60;
  DynamicJsonBuffer resBuffer(capacity);

  JsonObject& resRoot = resBuffer.parseObject(res);

  uint8_t sunrise_hour = resRoot["sunrise"]["hour"];
  uint8_t sunrise_minute = resRoot["sunrise"]["minute"];

  uint8_t sunset_hour = resRoot["sunset"]["hour"];
  uint8_t sunset_minute = resRoot["sunset"]["minute"];

  if (httpCode != 200) {
    DEBUG_PRINTLN("Setting fallback sunrise / sunset.");
    return String(6) + ":" + String(0) + "-" + String(18) + ":" + String(0);
  }
  else {
    return String(sunrise_hour) + ":" + String(sunrise_minute) + "-" + String(sunset_hour) + ":" + String(sunset_minute);
  }
}
