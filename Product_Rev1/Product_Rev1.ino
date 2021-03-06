// Pins for the GSM module
#define GSM_RX 19
#define GSM_TX 18

// Pins for sensor A
#define SEN1_ENB 22
#define SEN1_RST 24
#define SEN1_RX 17
#define SEN1_TX 16

// Pins for sensor B
#define SEN2_ENB 28
#define SEN2_RST 26
#define SEN2_RX 15
#define SEN2_TX 14

// Pins for sensor C
#define SEN3_RX 15
#define SEN3_TX 14

int TIMEOUT_GSM = 1000;   // Timeout period for GSM module

// Sensor types
int sen1 = 7003;
int sen2 = 5003;
int sen3 = 23;

// Data from sensors
int sen1_pm25 = 0;
int sen1_pm10 = 0;
int sen2_pm25 = 0;
int sen2_pm10 = 0;
int sen3_pm25 = 0;
int sen3_pm10 = 0;

void setup() {
  // Pin modes
  pinMode(SEN1_ENB, OUTPUT);
  pinMode(SEN1_RST, OUTPUT);
  pinMode(SEN2_ENB, OUTPUT);
  pinMode(SEN2_RST, OUTPUT);
  
  // Ports 
  digitalWrite(SEN1_ENB,HIGH);
  digitalWrite(SEN1_RST,HIGH);
  digitalWrite(SEN2_ENB,HIGH);
  digitalWrite(SEN2_RST,HIGH);

  // Serial port to computer
  Serial.begin(9600);
  //Serial.println("Initializing...");

  // Serial port to sensors 
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  
  // Settle down time
  delay(2000);
}

void loop() {

  int temp = sensorDataAvailable_1();
  if (temp == 2){
    sen1_pm25 = sen1_pm25 * 1;
    sen1_pm10 = sen1_pm10 * 1;
    
    Serial.print("A ");
    Serial.print(sen1_pm25, 1);
    Serial.print(" ");
    Serial.print(sen1_pm10, 1);
    Serial.println();
  } else if (temp == 1){
    //Serial.println("A CRC Error");
  } else if (temp == 0){
    //Serial.println("A Timeout");
  }

  delay(5);

  temp = sensorDataAvailable_2();
  if (temp == 2){
    sen2_pm25 = sen2_pm25 * 1;
    sen2_pm10 = sen2_pm10 * 1;
    
    Serial.print("B ");
    Serial.print(sen2_pm25, 1);
    Serial.print(" ");
    Serial.print(sen2_pm10, 1);
    Serial.println();
  } else if (temp == 1){
    //Serial.println("B CRC Error");
  } else if (temp == 0){
    //Serial.println("B Timeout");
  }

  delay(5);

  temp = sensorDataAvailable_3();
  if (temp == 2){
    sen3_pm25 = sen3_pm25 * 1;
    sen3_pm10 = sen3_pm10 * 1;
    
    Serial.print("C ");
    Serial.print(sen3_pm25, 1);
    Serial.print(" ");
    Serial.print(sen3_pm10, 1);
    Serial.println();
  } else if (temp == 1){
    //Serial.println("C CRC Error");
  } else if (temp == 0){
    //Serial.println("C Timeout");
  }
  
  delay(1000);

}

// Return 0 = Timeout, 1 = CRC Error, 2 = Success
int sensorDataAvailable_1(void){
  // Spin until we hear meassage header byte
  long startTime = millis();

  while (1){
    while (!Serial1.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; //Timeout error
    }

    if (Serial1.read() == 0x42) 
      break; // We have first part of header
  }
  
  while (1){
    while (!Serial1.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; // Timeout error
    }

    if (Serial1.read() == 0x4d) 
      break; // We have the complete message header
  }

  // Read the next 30 bytes
  byte sensorValue[30];
  for (byte spot = 0 ; spot < 30 ; spot++){
    startTime = millis();
    while (!Serial1.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; // Timeout error
    }
    sensorValue[spot] = Serial1.read();
    
  }
  
  // Check CRC
  int crc = 66 + 77;
  for (byte spot = 0 ; spot < 28 ; spot++){
    crc += sensorValue[spot];
  }
  if (crc != sensorValue[29] + 256 * sensorValue[28])
    return 1; // CRC error
  
  // Update the global variables
  if (sen1 = 5003){
    sen1_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen1_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  } else if (sen1 = 7003) {
    sen1_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen1_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  }
  return 2; // We've got a good reading!
}


// Return 0 = Timeout, 1 = CRC Error
int sensorDataAvailable_2(void){
  // Spin until we hear meassage header byte
  long startTime = millis();

  while (1){
    while (!Serial2.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; //Timeout error
    }

    if (Serial2.read() == 0x42) 
      break; // We have first part of header
  }
  
  while (1){
    while (!Serial2.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; // Timeout error
    }

    if (Serial2.read() == 0x4d) 
      break; // We have the complete message header
  }

  // Read the next 30 bytes
  byte sensorValue[30];
  for (byte spot = 0 ; spot < 30 ; spot++){
    startTime = millis();
    while (!Serial2.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; // Timeout error
    }
    sensorValue[spot] = Serial2.read();
  }
  
  // Check CRC
  int crc = 66 + 77;
  for (byte spot = 0 ; spot < 28 ; spot++){
    crc += sensorValue[spot];
  }
  if (crc != sensorValue[29] + 256 * sensorValue[28])
    return 1; // CRC error
  
  // Update the global variables
  if (sen2 = 5003){
    sen2_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen2_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  } else if (sen2 = 7003) {
    sen2_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen2_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  }
  return 2; // We've got a good reading!
}

// Return 0 = Timeout, 1 = CRC Error
int sensorDataAvailable_3(void){
  // Spin until we hear meassage header byte
  long startTime = millis();
        
  while (1){
    while (!Serial3.available()){
      delay(1);
      if (millis() - startTime > 1500){
        return 0; //Timeout error
      }
    }

    if (Serial3.read() == 0xAA) 
      break; // We have first part of header
  }
           
  // Read the next 10 bytes
  byte sensorValue[10];
  for (byte spot = 1 ; spot < 10 ; spot++){
    startTime = millis();
    while (!Serial3.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; // Timeout error
    }
    sensorValue[spot] = Serial3.read();
  }
 
  // Check CRC
  byte crc = 0;
  for (byte spot = 2 ; spot < 8 ; spot++){
    crc += sensorValue[spot];
  }
  if (crc != sensorValue[8])
    return 1; // CRC error
  
  // Update the global variables
  if (sen3 = 23) {
    sen3_pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
    sen3_pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;
  }
  return 2; // We've got a good reading!
}
