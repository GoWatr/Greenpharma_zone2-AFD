//Flowmeter.h

#include <SoftwareSerial.h>
SoftwareSerial Serial_port(16, 17);
const byte requestframe1[] = { 0x05, 0x03, 0x00, 0x00, 0x00, 0x21, 0x84, 0x56 };

extern SoftwareSerial Serial_port;
int V8_BIT_1, V8_2BIT_1 = 0;
int V8_BIT_2, V8_2BIT_2 = 0;
int V8_BIT_3 = 0;
uint64_t V16_BIT, V16_BIT1 = 0;
float Flow = 0.0;
String Tot;
int tot = 0;
uint8_t values[33];
int consecutiveFailures = 0;  // New counter variable

void readFlowMeterData(const byte requestFrame[], byte dataBuffer[]) {
  bool condition_met_FM = false;
  for (int attempt = 0; attempt < 3; attempt++) {
    memset(dataBuffer, 0, sizeof(dataBuffer));
    if (Serial_port.write(requestFrame, sizeof(requestframe1)) == 8) {
      delay(1000);
      Serial_port.flush();
      for (uint8_t i = 0; i < 32; i++) {
        dataBuffer[i] = Serial_port.read();
        delay(50);
      }
      Serial.println("---- Flowmeter ----");
      Serial.print("Received Data: ");
      for (uint8_t i = 0; i < 32; i++) {
        Serial.print(dataBuffer[i], HEX);
        Serial.print(' ');
      }
      Serial.println(' ');

      if ((dataBuffer[0] == 5) && (dataBuffer[1] == 3)) {
        condition_met_FM = true;
        consecutiveFailures = 0;
        break;
      }
    } else {
      Serial.println("Failed to send request to flow meter");
    }
    delay(1000);
  }
  if (!condition_met_FM) {
    for (uint8_t i = 0; i < 32; i++) {
      Serial.print(dataBuffer[i], HEX);
      Serial.print(' ');
    }
    consecutiveFailures++;
    if (consecutiveFailures >= 10) {
      Flow = -1;
      Tot = "-1";
      consecutiveFailures = 0;  // Reset the counter after sending -1
    }
    Serial.println("Failed to get valid data from flow meter after 5 attempts");
  }
}

void processFlowMeterData(byte dataBuffer[]) {
  if ((dataBuffer[0] == 5) && (dataBuffer[1] == 3)) {
    //FLOW
    V8_2BIT_1 = (dataBuffer[5] << 8) | dataBuffer[6];

    //Tot
    V8_BIT_1 = (dataBuffer[19] << 8) | dataBuffer[20];
    V8_BIT_2 = (dataBuffer[21] << 8) | dataBuffer[22];
    V16_BIT = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
    V8_BIT_3 = (dataBuffer[23] << 8) | dataBuffer[24];

    //final
    Flow = V8_2BIT_1 * (1000 / 60);
    Flow = Flow * 0.001;
    Tot = String(static_cast<uint32_t>(V16_BIT)) + "." + String(V8_BIT_3);

    Serial.println("Flow =" + String(Flow));
    Serial.println("Tot  =" + String(Tot));
  } else {
    Serial.println("Conduction not matched... Data not sending...");
  }
}

void Flowmeter() {
  byte dataBuffer[33];
  readFlowMeterData(requestframe1, dataBuffer);
  processFlowMeterData(dataBuffer);
  delay(30);
  Serial_port.flush();
}
