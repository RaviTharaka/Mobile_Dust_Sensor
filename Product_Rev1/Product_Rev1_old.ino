
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

int TIMEOUT_GSM = 1000;   // Timeout period for GSM module

// Sensor types
int sen1 = 7003;
int sen2 = 5003;

// Data from sensors
int sen1_pm25 = 0;
int sen1_pm10 = 0;
int sen2_pm25 = 0;
int sen2_pm10 = 0;

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
  Serial.println("Initializing...");

  // Serial port to sensors 
  Serial2.begin(9600);
  Serial3.begin(9600);

  // Initializing the GSM module
  Serial1.begin(9600);
  initializeGSM();

  

  // Settle down time
  delay(2000);
}

void loop() {

  int temp = sensorDataAvailable_1();
  if (temp == 2){
    sen1_pm25 = sen1_pm25 * 1;
    sen1_pm10 = sen1_pm10 * 1;
    
    Serial.print("A 2.5:");
    Serial.print(sen1_pm25, 1);
    Serial.print(" 10:");
    Serial.print(sen1_pm10, 1);
    Serial.println();
  } else if (temp == 1){
    Serial.println("A CRC Error");
  } else if (temp == 0){
    Serial.println("A Timeout");
  }

  delay(5);

  temp = sensorDataAvailable_2();
  if (temp == 2){
    sen2_pm25 = sen2_pm25 * 1;
    sen2_pm10 = sen2_pm10 * 1;
    
    Serial.print("B 2.5:");
    Serial.print(sen2_pm25, 1);
    Serial.print(" 10:");
    Serial.print(sen2_pm10, 1);
    Serial.println();
  } else if (temp == 1){
    Serial.println("B CRC Error");
  } else if (temp == 0){
    Serial.println("B Timeout");
  }

  delay(1000);

}

// Initializes the GSM module
void initializeGSM(){
  String response = "";

  // GSM echo mode off
  sendData("ATE0", TIMEOUT_GSM, 1);
  delay(1000);
  
  // GSM operation verification
  Serial.println("GSM Verifying...");
  while (1){
    response = sendData("AT+CFUN?", TIMEOUT_GSM, 1);
    if (response.equals("\r\n+CFUN: 1\r\n\r\nOK\r\n")){
      response = sendData("AT+CPIN?", TIMEOUT_GSM, 1);
      if (response.equals("\r\n+CPIN: READY\r\n\r\nOK\r\n")){
        Serial.println("GSM module functional");
        break;  
      }
    } 
    Serial.println("GSM module error!! Retrying...");
    delay(1000); 
  }
  
  // Register with the service provider
  Serial.println("Connection verifying...");
  while (1){
    response = sendData("AT+CREG?", TIMEOUT_GSM, 1);
    if (response.equals("\r\n+CREG: 0,1\r\n\r\nOK\r\n")){
      Serial.println("Connection established");
      break;
    }
    Serial.println("Connection error!! Retrying...");
    delay(1000); 
  }
 
  // Establish GPRS connection and get the time
  Serial.println("GPRS connecting...");
  while(1){
    if (connectGPRS()){
      break;
    } 
    Serial.println("Time syncing failed!! Retrying...");
    delay(1000);
  }

}

bool connectGPRS(){
  String response = sendData("AT+SAPBR=3,1,\"Contype\",\"GPRS\"",TIMEOUT_GSM, 1);
  if (response.equals("\r\nOK\r\n")){
    sendData("AT+SAPBR=1,1",TIMEOUT_GSM, 1);
    response = sendData("AT+SAPBR=2,1",TIMEOUT_GSM, 1);
      
    String tempStr = response.substring(0,14);
    if (tempStr.equals("\r\n+SAPBR: 1,1,")){
      Serial.println("GPRS Established");

      // Get the time
      Serial.println("Getting time via NTP...");
      response = sendData("AT+CNTPCID=1",TIMEOUT_GSM, 1);
      if (response.equals("\r\nOK\r\n")){
        response = sendData("AT+CNTP=\"pool.ntp.org\",22",TIMEOUT_GSM, 1);
        if (response.equals("\r\nOK\r\n")){
          response = sendData("AT+CNTP",TIMEOUT_GSM * 5, 1);
          if (response.equals("\r\nOK\r\n\r\n+CNTP: 1\r\n")){
            response = sendData("AT+CCLK?", TIMEOUT_GSM, 1);
            Serial.println("Time sync successful");
            return true;  
          }
        }
      }
    } 
  }
  return false; 
}

String sendData(String command, const int timeout, boolean debug){
  String response = "";    
  Serial1.println(command); 
  long int time = millis();
  int i = 0;  
   
  while( (time+timeout) > millis()){
    while(Serial1.available()){       
      char c = Serial1.read();
      response+=c;
    }  
  }    
  if(debug){
    Serial.print(response);
  }    
    return response;
  }

// Return 0 = Timeout, 1 = CRC Error, 2 = Success
int sensorDataAvailable_1(void){
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
    while (!Serial3.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; //Timeout error
    }

    if (Serial3.read() == 0x42) 
      break; // We have first part of header
  }
  
  while (1){
    while (!Serial3.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; // Timeout error
    }

    if (Serial3.read() == 0x4d) 
      break; // We have the complete message header
  }

  // Read the next 30 bytes
  byte sensorValue[30];
  for (byte spot = 0 ; spot < 30 ; spot++){
    startTime = millis();
    while (!Serial3.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return 0; // Timeout error
    }
    sensorValue[spot] = Serial3.read();
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
