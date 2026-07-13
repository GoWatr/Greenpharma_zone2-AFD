#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>

WiFiClient wifiClient1;

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void otaUpdate(String updateURL) {
  HTTPClient http;
  WiFiClient client1;
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);

  t_httpUpdate_return ret = httpUpdate.update(client1, updateURL);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      ESP.restart();
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

void check_for_updates() {
  Serial.print("Current Firmware - ");
  Serial.print(CurrentVersion);
  Serial.println(" Checking for Updates ...");

  HTTPClient http;
  http.begin(wifiClient1, "http://gowatr.com/wms/ota-gowatr/ots_greenPharma/zone-2/firmware_zone2.json");

  int httpCode = http.GET();
  String payload = "";
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Received payload: " + payload);
  } else {
    Serial.printf("HTTP GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return;
  }
  http.end();

  String json = "";
  DynamicJsonDocument doc(2048);
  json = payload.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
  if (doc["ver"] > CurrentVersion && doc["ver"] != CurrentVersion) {
    Serial.println("New Firmware Available. Downloading new Firmware ... ");
    otaUpdate(doc["link"]);
  } else {
    Serial.println("Already Running Latest Firmware.");
  }
}
