//Ethernet.h

#include <ESPping.h> // by dvarrel ver:1.0.4
#include <ESP32-ENC28J60.h> // by tobozo ver:1.0.1

#define SPI_HOST 1
#define SPI_CLOCK_MHZ 8
#define INT_GPIO 15
#define MISO_GPIO 19
#define MOSI_GPIO 23
#define SCLK_GPIO 18
#define CS_GPIO 5

static bool eth_connected = false;
//192.9.201.187  192.9.101.12  192.168.2.1  192.168.1.21
IPAddress local_IP(192, 168, 2, 90);  // Enter your desired static IP address
IPAddress gateway(192, 168, 2, 1);     // Enter your gateway IP address
IPAddress subnet(255, 255, 255, 0);    // Enter your subnet mask
IPAddress dns(8, 8, 8, 8);

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      delay(100);
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

HTTPClient http;
WiFiClientSecure client;

