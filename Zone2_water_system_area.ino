#define LED_BUILTIN 2
#define BUTTON_PIN 34
#define CurrentVersion 2.0
#define motor1_relay 25
#define motor2_relay 33
#define valve1 2
#define valve2 4
#define valve3 13

#include "Datasend.h"
#include "Ethernet.h"
#include "Flowmeter.h"
#include "Hydrostatic_sensors.h"
#include "OTAupdate.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long previousDisplayTime = 0;
int currentScreen, previousDataSendTime = 0;
unsigned long displayInterval = 30000;
unsigned long dataSendInterval = 60000;
const unsigned long checkverupdate = 300000;
static unsigned long lastUpdateCheck = 0;
#define maxConnectionAttempts 10

uint8_t ping_flag, button_state, button_count = 0, eth_flag;
int ping_fail_counter = 0, i, counter;

//TIMER INTERRUPT
hw_timer_t* My_timer = NULL;
void IRAM_ATTR onTimer() {
  button_state = digitalRead(BUTTON_PIN);
  if (button_count < 3) {
    if (digitalRead(BUTTON_PIN) == 1) {
      button_count++;
    }
  } else if (button_count == 3) {
    button_count = 0;
  }
  ping_flag == 1;
}
const char* MyHostName = "ESP32-Test";

void setup() {
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer);  //Just Enable

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(motor1_relay, OUTPUT);
  pinMode(motor2_relay, OUTPUT);
  pinMode(valve1, OUTPUT);
  pinMode(valve2, OUTPUT);
  pinMode(valve3, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(motor1_relay, LOW);
  digitalWrite(motor2_relay, LOW);
  digitalWrite(valve1, LOW);
  digitalWrite(valve2, LOW);
  digitalWrite(valve3, LOW);
  digitalWrite(valve1, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(7, 0);
  lcd.print("GOWATR");
  lcd.setCursor(4, 1);
  lcd.print("SMART WATER");
  lcd.setCursor(1, 2);
  lcd.print("MANAGEMENT  SYSTEM");
  lcd.setCursor(12, 3);
  lcd.print("VER-1.1");
  lcd.setCursor(16, 3);
  lcd.print(CurrentVersion);
  delay(4000);

  WiFi.onEvent(WiFiEvent);
  ETH.begin(MISO_GPIO, MOSI_GPIO, SCLK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, SPI_HOST);
  delay(100);
  ETH.config(local_IP, gateway, subnet, dns);
  delay(100);
  while (!eth_connected) {
    i++;
    delay(1000);
    Serial.println("Waiting for Ethernet connection");
    if (i == 20) {
      break;
    }
  }
  Serial_port.begin(9600);
  lcd.clear();
  lcd.setCursor(3, 1);  // move cursor the first row
  lcd.print("SYSTEM INIT...");
}
void loop() {
  ping();
  if (!eth_connected) {
    Serial.println("Ethernet connection lost. Trying to reconnect...");

    ETH.begin(MISO_GPIO, MOSI_GPIO, SCLK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, SPI_HOST);
    delay(100);
    ETH.config(local_IP, gateway, subnet, dns);
    delay(100);
    int attempts = 0;
    while (!eth_connected && attempts < maxConnectionAttempts) {
      delay(1000);  // Wait for connection
      Serial.print(".");
      attempts++;
    }
    //Serial_port.begin(9600);
    if (eth_connected) {
      Serial.println("\nConnected to Ethernet successfully.");
    } else {
      Serial.println("\nFailed to reconnect to Ethernet.");
    }
  } else {
    ping_fail_counter = 0;
    Serial.println("Ethernet connection is already established.");
  }

  Flowmeter();
  Serial_port.flush();
  delay(1000);
  hydro_sensor();
  Serial_port.flush();
  delay(1000);
  valve_control();

  Serial.print("MF2WS TO SWST(TER) tank in mm :  " + String(hydro1_mm));
  Serial.print("\t MF2WS TO SWST(TER) tank in %  :  " + String(hydro1_per));
  Serial.println("\t\t The status of Valve1 is " + String(Valve1_status));
  Serial.print("PDST-02KL-TER tank in mm      :  " + String(hydro2_mm));
  Serial.print("\t PDST-02KL-TER tank in %       :  " + String(hydro2_per));
  Serial.println("\t\t The status of Valve2 is " + String(Valve2_status));
  Serial.print("CHST-02KL-TER tank in mm      :  " + String(hydro3_mm));
  Serial.print("\t CHST-02KL-TER tank in %       :  " + String(hydro3_per));
  Serial.println("\t\t The status of Valve3 is " + String(Valve3_status));
  Serial.print("WSST-05KL-WRS tank in mm      :  " + String(hydro4_mm));
  Serial.println("\t WSST-05KL-WRS tank  %         :  " + String(hydro4_per));
  delay(1000);

  unsigned long currentTime_sendData = millis();
  if (currentTime_sendData - previousDataSendTime >= dataSendInterval) {
    previousDataSendTime = currentTime_sendData;
    sendDataToServer();
  }
  lcd_display();

  unsigned long currentMillis_update = millis();
  if (currentMillis_update - lastUpdateCheck >= checkverupdate) {
    check_for_updates();  // Check for updates every hour
    lastUpdateCheck = currentMillis_update;
  }
  Serial.println(" ");
}

void sendDataToServer() {
  datasend = ID1 + ID2 + String(Tot) + ID3 + String(Flow) + ID4 + String(hydro1_mm) + ID5 + String(hydro1_per) + ID6 + String(hydro2_mm) + ID7 + String(hydro2_per)
             + ID8 + String(hydro3_mm) + ID9 + String(hydro3_per) + ID10 + String(hydro4_mm) + ID11 + String(hydro4_per) + ID12 + String(Valve1_status) + ID13 + String(Valve2_status)
             + ID14 + String(Valve3_status) + ID15 + String(Motor1_status) + ID16 + String(Motor2_status);
  sendToServer(datasend);
  Serial.println(datasend);
}

void lcd_display() {
  unsigned long currentTime = millis();

  if (currentTime - previousDisplayTime >= displayInterval) {
    previousDisplayTime = currentTime;

    switch (currentScreen) {
      case 0:
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("WSST-05KL-WRS");
        lcd.setCursor(0, 1);
        lcd.print("LEVEL : ");
        lcd.setCursor(8, 1);
        lcd.print(hydro4_per);
        lcd.print(" %");
        lcd.setCursor(0, 2);
        lcd.print("VALVE : ");
        lcd.setCursor(8, 2);
        lcd.print(Valve4_status == 1 ? "ON " : "OFF");
        lcd.setCursor(2, 3);
        lcd.print("ETH STATUS: ");
        eth_status_disp();
        break;

      case 1:
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("CHST-02KL-TER");
        lcd.setCursor(0, 1);
        lcd.print("LEVEL : ");
        lcd.setCursor(8, 1);
        lcd.print(hydro3_per);
        lcd.print(" %");
        lcd.setCursor(0, 2);
        lcd.print("VALVE : ");
        lcd.setCursor(8, 2);
        lcd.print(Valve3_status == 1 ? "ON " : "OFF");
        lcd.setCursor(2, 3);
        lcd.print("ETH STATUS: ");
        eth_status_disp();
        break;

      case 2:
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("PDST-02KL-TER");
        lcd.setCursor(0, 1);
        lcd.print("LEVEL : ");
        lcd.setCursor(8, 1);
        lcd.print(hydro2_per);
        lcd.print(" %");
        lcd.setCursor(0, 2);
        lcd.print("VALVE : ");
        lcd.setCursor(8, 2);
        lcd.print(Valve2_status == 1 ? "ON " : "OFF");
        lcd.setCursor(2, 3);
        lcd.print("ETH STATUS: ");
        eth_status_disp();
        break;

      case 3:
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("FCST-05KL-TER");
        lcd.setCursor(0, 1);
        lcd.print("LEVEL : ");
        lcd.setCursor(8, 1);
        lcd.print(hydro1_per);
        lcd.print(" %");
        lcd.setCursor(0, 2);
        lcd.print("VALVE : ");
        lcd.setCursor(8, 2);
        lcd.print(Valve1_status == 1 ? "ON " : "OFF");
        lcd.setCursor(2, 3);
        lcd.print("ETH STATUS: ");
        eth_status_disp();
        break;

      case 4:
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("MF2WS TO SWST(TER)");
        lcd.setCursor(0, 1);
        lcd.print("F~ : ");
        lcd.setCursor(6, 1);
        lcd.print(Flow);
        lcd.print("L/min");
        lcd.setCursor(0, 2);
        lcd.print("T+ : ");
        lcd.setCursor(6, 2);
        lcd.print(Tot);
        lcd.setCursor(2, 3);
        lcd.print("ETH STATUS: ");
        eth_status_disp();
        break;

      case 5:
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("MOTOR1 STATUS: ");
        lcd.setCursor(14, 1);
        lcd.print(Motor1_status == 1 ? "ON " : "OFF");
        lcd.setCursor(0, 2);
        lcd.print("MOTOR2 STATUS: ");
        lcd.setCursor(14, 2);
        lcd.print(Motor2_status == 1 ? "ON " : "OFF");
        lcd.setCursor(2, 3);
        lcd.print("ETH STATUS: ");
        eth_status_disp();
        break;

      default:
        break;
    }

    currentScreen++;
    if (currentScreen > 5) {  // 5 is the total number of screens
      currentScreen = 0;
    }
  }
}

void ping() {
  const IPAddress remote_ip(8, 8, 8, 8);
  Serial.print("Pinging ");
  Serial.print(remote_ip);
  if (Ping.ping(remote_ip) > 0) {
    Serial.print(" Ping success ");
    Serial.printf(" response time : %d/%.2f/%d ms\n", Ping.minTime(), Ping.averageTime(), Ping.maxTime());
    eth_flag = 1;
    delay(1000);
  } else {
    Serial.println("Server not reacheble - Ping failed");
    ping_fail_counter++;
    delay(1000);
    eth_flag = 0;
    if (ping_fail_counter >= 100) {
      ESP.restart();
    }
  }
  delay(100);
}

void hydro_sensor() {
  delay(2000);
  hs_sensor1();
  delay(3000);
  hs_sensor2();
  delay(3000);
  hs_sensor3();
  delay(3000);
  hs_sensor4();
  delay(3000);
}

void valve_control() {
  // Valve 1 control
  if ((hydro1_per <= 60) && (hydro1 != -1) && (Hydro1_byte[0] == 1)) {
    digitalWrite(valve1, HIGH);  // Open valve 1
    Valve1_status = 1;
  } else if ((hydro1_per >= 85 && (Hydro1_byte[0] == 1)) || (hydro1 == -1)) {
    // digitalWrite(valve1, LOW);  // Close valve 1
    Valve1_status = 0;
  }
  // Valve 2 control
  if ((hydro2_per <= 60) && (hydro2 != -1) && (Hydro2_byte[0] == 2)) {
    digitalWrite(valve2, HIGH);  // Open valve 2
    Valve2_status = 1;
  } else if ((hydro2_per >= 85 && (Hydro2_byte[0] == 2)) || (hydro2 == -1)) {
    //digitalWrite(valve2, LOW);  // Close valve 2
    Valve2_status = 0;
  }
  // Valve 3 control
  if ((hydro3_per <= 60) && (hydro3 != -1) && (Hydro3_byte[0] == 3)) {
    digitalWrite(valve3, HIGH);  // Open valve 3
    Valve3_status = 1;
  } else if ((hydro3_per >= 85 && (Hydro3_byte[0] == 3)) || (hydro3 == -1)) {
    //digitalWrite(valve3, LOW);  // Close valve 3
    Valve3_status = 0;
  }

  delay(2000);

  // Motor control based on tank levels and sensor status
  if ((Valve1_status == 1 || Valve2_status == 1 || Valve3_status == 1) && (hydro4_per >= 40 && (Hydro4_byte[0] == 4))) {
    // Turn on motor if any of the first three tanks are below 60% and the fourth tank is above 40%
    digitalWrite(motor1_relay, HIGH);
    digitalWrite(motor2_relay, HIGH);
    Motor1_status = 1;
    Motor2_status = 1;
  } else if (hydro4_per <= 20) {
    // Turn off motor if all tanks are above 90% or there is a sensor failure in any tank or valve off statis, or if the fourth tank is below 10%
    digitalWrite(motor1_relay, LOW);
    digitalWrite(motor2_relay, LOW);
    Motor1_status = 0;
    Motor2_status = 0;
  } else if (Valve1_status == 0 && Valve2_status == 0 && Valve3_status == 0) {
    // Turn off motor if all tanks are above 90% or there is a sensor failure in any tank or valve off statis, or if the fourth tank is below 10%
    digitalWrite(motor1_relay, LOW);
    digitalWrite(motor2_relay, LOW);
    Motor1_status = 0;
    Motor2_status = 0;
  }

  delay(2000);

  if ((hydro1_per >= 85) || (hydro1 == -1)) {
    digitalWrite(valve1, LOW);  // Close valve 1
    Valve1_status = 0;
  }
  if ((hydro2_per >= 85) || (hydro2 == -1)) {
    digitalWrite(valve2, LOW);  // Close valve 2
    Valve2_status = 0;
  }
  if ((hydro3_per >= 85) || (hydro3 == -1)) {
    digitalWrite(valve3, LOW);  // Close valve 3
    Valve3_status = 0;
  }
}

// void valve_control() {
//   // Valve 1 control
//   if ((hydro1_per <= 60) && (hydro1 != -1) && (Hydro1_byte[0] == 1)) {
//     digitalWrite(valve1, HIGH);  // Open valve 1
//     Valve1_status = 1;
//   } else if ((hydro1_per >= 85 && (Hydro1_byte[0] == 1)) || (hydro1 == -1)) {
//     digitalWrite(valve1, LOW);  // Close valve 1
//     Valve1_status = 0;
//   }
//   // Valve 2 control
//   if ((hydro2_per <= 60) && (hydro2 != -1) && (Hydro2_byte[0] == 2)) {
//     digitalWrite(valve2, HIGH);  // Open valve 2
//     Valve2_status = 1;
//   } else if ((hydro2_per >= 85 && (Hydro2_byte[0] == 2)) || (hydro2 == -1)) {
//     digitalWrite(valve2, LOW);  // Close valve 2
//     Valve2_status = 0;
//   }
//   // Valve 3 control
//   if ((hydro3_per <= 60) && (hydro3 != -1) && (Hydro3_byte[0] == 3)) {
//     digitalWrite(valve3, HIGH);  // Open valve 3
//     Valve3_status = 1;
//   } else if ((hydro3_per >= 85 && (Hydro3_byte[0] == 3)) || (hydro3 == -1)) {
//     digitalWrite(valve3, LOW);  // Close valve 3
//     Valve3_status = 0;
//   }

//   delay(2000);

//   // Motor control based on tank levels and sensor status
//   if ((Valve1_status == 1 || Valve2_status == 1 || Valve3_status == 1) && (hydro4_per >= 40 && (Hydro4_byte[0] == 4))) {
//   // Turn on motor if any of the first three tanks are below 60% and the fourth tank is above 40%
//   digitalWrite(motor1_relay, HIGH);
//   digitalWrite(motor2_relay, HIGH);
//   Motor1_status = 1;
//   Motor2_status = 1;
//   } else if (hydro4_per <= 20) {
//   // Turn off motor if all tanks are above 90% or there is a sensor failure in any tank or valve off statis, or if the fourth tank is below 10%
//   digitalWrite(motor1_relay, LOW);
//   digitalWrite(motor2_relay, LOW);
//   Motor1_status = 0;
//   Motor2_status = 0;
//   } else if (Valve1_status == 0 && Valve2_status == 0 && Valve3_status == 0) {
//   // Turn off motor if all tanks are above 90% or there is a sensor failure in any tank or valve off statis, or if the fourth tank is below 10%
//   digitalWrite(motor1_relay, LOW);
//   digitalWrite(motor2_relay, LOW);
//   Motor1_status = 0;
//   Motor2_status = 0;
//   }
// }

void eth_status_disp() {
  if (eth_flag == 1) {
    lcd.setCursor(14, 3);  // move cursor to the third row
    lcd.print("OKAY");
  } else {
    lcd.setCursor(14, 3);  // move cursor to the third row
    lcd.print("FAIL");
  }
}