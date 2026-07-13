//Hydrostatic_sensors.h
const byte Hydro_1[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0F, 0x05, 0xCE };
const byte Hydro_2[] = { 0x02, 0x03, 0x00, 0x00, 0x00, 0x0F, 0x05, 0xFD };
const byte Hydro_3[] = { 0x03, 0x03, 0x00, 0x00, 0x00, 0x0F, 0x04, 0x2C };
const byte Hydro_4[] = { 0x04, 0x03, 0x00, 0x00, 0x00, 0x0F, 0x05, 0x9B };

int16_t hydro1, hydro2, hydro3, hydro4 = 0;
float hydro1_per, hydro2_per, hydro3_per, hydro4_per = 0.0;
int16_t hydro1_mm, hydro2_mm, hydro3_mm, hydro4_mm = 0;

byte Hydro1_byte[128], Hydro2_byte[128], Hydro3_byte[128], Hydro4_byte[128];
int Motor1_status, Motor2_status, Valve1_status, Valve2_status, Valve3_status, Valve4_status = 0;


//********************tank_1********************
void hs_sensor1() {
  bool condition_met = false; // Flag to track if condition is met
  
  for (int attempt = 0; attempt < 3; attempt++) {
    if (Serial_port.write(Hydro_1, sizeof(Hydro_1)) == 8) {
      Serial_port.flush();
      delay(1000);
      for (byte i = 0; i <= 14; i++) {
        Hydro1_byte[i] = Serial_port.read();
        delay(10);
      }
      Serial.print("Tank1 Received BYTES: ");
      for (byte i = 0; i <= 14; i++) {
        Serial.print(Hydro1_byte[i], HEX);
        Serial.print(' ');
      }
      Serial.println(" ");
      
      if ((Hydro1_byte[0] == 1) && (Hydro1_byte[1] == 3)) { // Check point
        hydro1 = (Hydro1_byte[7] << 8) | Hydro1_byte[8];
        hydro1_per = hydro1 * 0.1;
        hydro1_mm = (Hydro1_byte[3] << 8) | Hydro1_byte[4];  // Tank level in millimeter
        condition_met = true; 
        break; 
      }
    }
    delay(1000);
    
    if (condition_met) 
      break; 
  }
  if (!condition_met) {
    Serial.println("Failed to get valid data from tank 1 sensor after 5 attempts");
    hydro1 = -1;
    hydro1_per = -1; 
    hydro1_mm = -1;
  }
}

//********************tank_2********************
void hs_sensor2() {
  bool condition_met = false; // Flag to track if condition is met
  
  for (int attempt = 0; attempt < 3; attempt++) {
    if (Serial_port.write(Hydro_2, sizeof(Hydro_2)) == 8) {
      delay(1000);
      Serial_port.flush();
      for (byte i = 0; i <= 14; i++) {
        Hydro2_byte[i] = Serial_port.read();
        delay(10);
      }
      Serial.print("Tank2 Received BYTES: ");
      for (byte i = 0; i <= 14; i++) {
        Serial.print(Hydro2_byte[i], HEX);
        Serial.print(' ');
      }
      Serial.println(" ");
      
      if ((Hydro2_byte[0] == 2) && (Hydro2_byte[1] == 3)) { // Check point
        hydro2 = (Hydro2_byte[7] << 8) | Hydro2_byte[8];
        hydro2_per = hydro2 * 0.1;
        hydro2_mm = (Hydro2_byte[3] << 8) | Hydro2_byte[4];  // Tank level in millimeter
        condition_met = true;
        break;
      }
    }
    delay(1000);
    
    if (condition_met)
      break;
  }
  if (!condition_met) {
    Serial.println("Failed to get valid data from tank 2 sensor after 5 attempts");
    hydro2 = -1;
    hydro2_per = -1; 
    hydro2_mm = -1;
  }
}

//********************tank_3********************
void hs_sensor3() {
  bool condition_met = false; // Flag to track if condition is met
  
  for (int attempt = 0; attempt < 3; attempt++) {
    if (Serial_port.write(Hydro_3, sizeof(Hydro_3)) == 8) {
      delay(1000);
      Serial_port.flush();
      for (byte i = 0; i <= 14; i++) {
        Hydro3_byte[i] = Serial_port.read();
        delay(10);
      }
      Serial.print("Tank3 Received BYTES: ");
      for (byte i = 0; i <= 14; i++) {
        Serial.print(Hydro3_byte[i], HEX);
        Serial.print(' ');
      }
      Serial.println(" ");
      
      if ((Hydro3_byte[0] == 3) && (Hydro3_byte[1] == 3)) { // Check point
        hydro3 = (Hydro3_byte[7] << 8) | Hydro3_byte[8];
        hydro3_per = hydro3 * 0.1;
        hydro3_mm = (Hydro3_byte[3] << 8) | Hydro3_byte[4];  // Tank level in millimeter
        condition_met = true; // Set the flag to true
        break; // Exit the retry loop if condition is met
      }
    }
    delay(1000);
    if (condition_met)
      break;
  }
  if (!condition_met) {
    Serial.println("Failed to get valid data from tank 3 sensor after 5 attempts");
    hydro3 = -1;
    hydro3_per = -1; 
    hydro3_mm = -1;
  }
}

//********************tank_4 2nd floor tank********************
void hs_sensor4() {
  bool condition_met = false;
  
  for (int attempt = 0; attempt < 5; attempt++) {
    if (Serial_port.write(Hydro_4, sizeof(Hydro_4)) == 8) {
      delay(1000);
      Serial_port.flush();
      for (byte i = 0; i <= 14; i++) {
        Hydro4_byte[i] = Serial_port.read();
        delay(10);
      }
      Serial.print("Tank4 Received BYTES: ");
      for (byte i = 0; i <= 14; i++) {
        Serial.print(Hydro4_byte[i], HEX);
        Serial.print(' ');
      }
      Serial.println(" ");
      
      if ((Hydro4_byte[0] == 4) && (Hydro4_byte[1] == 3)) { // Check point
        hydro4 = (Hydro4_byte[7] << 8) | Hydro4_byte[8];
        hydro4_per = hydro4 * 0.1;
        hydro4_mm = (Hydro4_byte[3] << 8) | Hydro4_byte[4];  // Tank level in millimeter
        condition_met = true;
        break;
      }
    }
    delay(1000);
    if (condition_met)
      break; 
  }
  if (!condition_met) {
    Serial.println("Failed to get valid data from tank 4 sensor after 5 attempts");
    hydro4 = -1;
    hydro4_per = -1; 
    hydro4_mm = -1;
  }
}
