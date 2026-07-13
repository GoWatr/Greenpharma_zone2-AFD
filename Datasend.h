//Datasend.h
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "Def_Cons.h"

void sendToServer(String data) {
  HTTPClient http;
  String url = String(serverUrl) + data;
  http.begin(url);

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.println("HTTP Request failed");
  }
  http.end();
}


