// Libraries
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

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

// Pins for LCD display
#define DISP_SCLK 9
#define DISP_MISO 8
#define DISP_MOSI 10
#define DISP_CS 13
#define DISP_DC 11
#define DISP_RST 12

int TOTAL_READING = 10;   // Number of averaged readings
int CORR_READING = 5;     // Number of valid readings required

int TIMEOUT_SEN = 1500;   // Timeout period for sensor reading
int TIMEOUT_GSM = 1000;   // Timeout period for GSM module
int TEXT_SIZE = 1;        // Display text size

int SEN_A_ID = 10003;     // Unique ID for sensor A
int SEN_B_ID = 10004;     // Unique ID for sensor B

// Sensor types
int sen1 = 7003;
int sen2 = 5003;

// Data from sensors
int sen1_pm25 = 0;
int sen1_pm10 = 0;
int sen2_pm25 = 0;
int sen2_pm10 = 0;

// Averaged data from sensors
int sen1_pm10_avg = 0;
int sen1_pm25_avg = 0;
int sen2_pm10_avg = 0;
int sen2_pm25_avg = 0;
int sen1_corr = 0;
int sen2_corr = 0;

// Date and time
int year = 0;
int month = 0;
int day = 0;
int hour = 0;
int minute = 0;
int second = 0;

// Initialize display
Adafruit_ILI9340 disp = Adafruit_ILI9340(DISP_CS, DISP_DC, DISP_MOSI, DISP_SCLK, DISP_RST, DISP_MISO);
int textline = 0;

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

  // SPI port to display
  disp.begin();
  disp.fillScreen(ILI9340_BLACK);
  disp.setRotation(1);
  disp.setCursor(0, 0);
  
  // Serial port to computer
  Serial.begin(9600);
  displayMessage("Initialzing...", TEXT_SIZE);  

  // Serial port to sensors 
  Serial2.begin(9600);
  Serial3.begin(9600);

  // Initializing the GSM module
  Serial1.begin(9600);
  initializeGSM();

  // Settle down
  delay(6000);
  
}

void loop() {
  int sen1_status[TOTAL_READING];
  sen1_pm10_avg = 0;
  sen1_pm25_avg = 0;
  sen1_corr = 0;

  int sen2_status[TOTAL_READING];
  sen2_pm10_avg = 0;
  sen2_pm25_avg = 0;
  sen2_corr = 0;

  displayMessage("Collecting data from sensors...", TEXT_SIZE);
  
  for (int i = 0; i < TOTAL_READING; i++) {
    sen1_status[i] = sensorDataAvailable_1();
    sen2_status[i] = sensorDataAvailable_2();
    
    if (sen1_status[i] == 2) {
      sen1_pm10_avg = sen1_pm10_avg + sen1_pm10 * 1;
      sen1_pm25_avg = sen1_pm25_avg + sen1_pm25 * 1;
      sen1_corr = sen1_corr + 1;
    }

    if (sen2_status[i] == 2) {
      sen2_pm10_avg = sen2_pm10_avg + sen2_pm10 * 1;
      sen2_pm25_avg = sen2_pm25_avg + sen2_pm25 * 1;
      sen2_corr = sen2_corr + 1;
    }
  }

  // Display Time
  while (!getTime()) {
    initializeGSM();
  }
  
  if (sen1_corr > CORR_READING) {
    sen1_pm25_avg = sen1_pm25_avg / sen1_corr;
    sen1_pm10_avg = sen1_pm10_avg / sen1_corr;
    
    String text = "SenA - PM2.5:";
    text = text + sen1_pm25_avg + "\r\n       PM10:" + sen1_pm10_avg + "\r\n       smpl:" + sen1_corr;
    displayMessage(text , 2);
  } else {
    String text = "SenA : Only ";
    text = text + sen1_corr + " data is correct";
    displayMessage(text , 2);
    text = "Log : ";
    for (int i = 0; i < TOTAL_READING; i++) {
      text = text + sen1_status[i]; 
    }
    displayMessage(text, 2);
  }

  if (sen2_corr > CORR_READING) {
    sen2_pm25_avg = sen2_pm25_avg / sen2_corr;
    sen2_pm10_avg = sen2_pm10_avg / sen2_corr;
    
    String text = "SenB - PM2.5:";
    text = text + sen2_pm25_avg + "\r\n       PM10:" + sen2_pm10_avg + "\r\n       smpl:" + sen2_corr;
    displayMessage(text , 2);
  } else {
    String text = "SenB : Only ";
    text = text + sen2_corr + " data is correct";
    displayMessage(text , 2);
    text = "Log : ";
    for (int i = 0; i < TOTAL_READING; i++) {
      text = text + sen2_status[i]; 
    }
    displayMessage(text, 2);
  }

  // Sending data to server
  if (sen1_corr > CORR_READING) {
    displayMessage("Sensor 1 sending to server...", TEXT_SIZE);
    for (int i = 0; i < 3; i++) {
      if (sendDataToServer(SEN_A_ID ,sen1_pm10_avg, sen1_pm25_avg, sen1_corr)){
        displayMessage("Data 1 sent to server!!", TEXT_SIZE);
        break;
      } else {
        displayMessage("Server connection failed, retrying...", TEXT_SIZE);
        delay(1000);
        initializeGSM();
      }
    }
  }

  if (sen1_corr > CORR_READING) {
    displayMessage("Sensor 2 sending to server...", TEXT_SIZE);
    for (int i = 0; i < 3; i++) {
      if (sendDataToServer(SEN_B_ID ,sen2_pm10_avg, sen2_pm25_avg, sen2_corr)){
        displayMessage("Data 2 sent to server!!", TEXT_SIZE);
        break;
      } else {
        displayMessage("Server connection failed, retrying...", TEXT_SIZE);
        delay(1000);
        initializeGSM();
      }
    }
  }
  
  delay(10000);
  disp.fillScreen(ILI9340_GREEN);
  disp.setCursor(0, 0);
  textline = 0;
   
}

// Sending data to server
bool sendDataToServer(int index, int pm10, int pm25, int samp) {
  String response = "";

  // Initialize HTTP connection
  response = sendData("AT+HTTPINIT", TIMEOUT_GSM, 1);
  delay(1000);
  response = sendData("AT+HTTPPARA=\"CID\",1", TIMEOUT_GSM, 1);
  if (response.equals("\r\nOK\r\n")){
    // Set the URL
    String command = "AT+HTTPPARA=\"URL\",\"http://www.yung.lk/airquality/record.php?id=";
    command = command + index + "&battery=34&longitude=" + samp + "&latitude=" + samp + "&data={ \\\"datapoints\\\": [ { \\\"time\\\": \\\"";
    command = command + hour + ":" + minute + ":" + second + "T" + day + ":" + month + ":" + year + "\\\", \\\"pm10\\\": ";
    command = command + pm10 + ", \\\"pm25\\\" : " + pm25 +" }]}\"";
    
    response = sendData(command, TIMEOUT_GSM, 1);
    
    if (response.equals("\r\nOK\r\n")){
      displayMessage("URL accepted", TEXT_SIZE);

      // Send data to server
      response = sendData("AT+HTTPACTION=0", 5*TIMEOUT_GSM, 1);
      String temp = response.substring(0,27);
      if (temp.equals("\r\nOK\r\n\r\n+HTTPACTION: 0,200,")) {
        response = sendData("AT+HTTPREAD", TIMEOUT_GSM, 1);
        if (response.equals("\r\n+HTTPREAD: 8\r\nOKAY<br>\r\nOK\r\n")) {
          return 1;
        }
      }
    }
  }
  return 0;
}

// Initializes the GSM module
void initializeGSM(){
  disp.fillScreen(ILI9340_BLACK);
  disp.setCursor(0, 0);
  textline = 0;
  
  String response = "";

  // GSM operation verification
  displayMessage("GSM Verifying...", TEXT_SIZE);
  while (1){
    // GSM echo mode off
    sendData("ATE0", TIMEOUT_GSM, 1);
    delay(1000);

    // POST on GSM module
    response = sendData("AT+CFUN?", TIMEOUT_GSM, 1);
    if (response.equals("\r\n+CFUN: 1\r\n\r\nOK\r\n")){
      // SIM functional verification
      response = sendData("AT+CPIN?", TIMEOUT_GSM, 1);
      if (response.equals("\r\n+CPIN: READY\r\n\r\nOK\r\n")){
        displayMessage("GSM module functional", TEXT_SIZE);
        
        // Register with the service provider
        displayMessage("Connecting...", TEXT_SIZE);
        for (int i = 0; i < 3; i++) {
          response = sendData("AT+CREG?", TIMEOUT_GSM, 1);
          if (response.equals("\r\n+CREG: 0,1\r\n\r\nOK\r\n")){
            displayMessage("Connection established", TEXT_SIZE);
            
            // Establish GPRS connection and get the time
            displayMessage("GPRS connecting...", TEXT_SIZE);
            for (int j = 0; j < 3; j++) {
              if (connectGPRS() & getTime()) {
                displayMessage("GSM functional!!", TEXT_SIZE);
                delay(1000);

                // Refreshing display
                disp.fillScreen(ILI9340_GREEN);
                disp.setCursor(0, 0);
                textline = 0; 
                return; 
              } else {
                displayMessage("Time syncing failed!! Retrying...", TEXT_SIZE);
                delay(1000);
              }
            }
          } else {
            displayMessage("Connection error!! Retrying...", TEXT_SIZE);
            delay(1000); 
          }
        }
      }
    } 
    displayMessage("GSM module error!! Retrying...", TEXT_SIZE);
    delay(1000); 
  }
}

bool getTime(){
  displayMessage("Getting time...", TEXT_SIZE);
  
  String response = sendData("AT+CCLK?", TIMEOUT_GSM, 1);

  if (response.substring(0,9).equals("\r\n+CCLK: ")){
    year   = response.substring(10, 12).toInt();
    month  = response.substring(13, 15).toInt();
    day    = response.substring(16, 18).toInt();
    hour   = response.substring(19, 21).toInt();
    minute = response.substring(22, 24).toInt();
    second = response.substring(25, 27).toInt();
    
    String text = "";
    text = text + "Time " + hour + ":" + minute + ":" + second + " on " + year + "/" + month + "/" + day;
    displayMessage(text, TEXT_SIZE);
    return true;
  } else {
    displayMessage("Time unavailable", TEXT_SIZE);
    return false;
  }
  
}

bool connectGPRS(){
  String response = sendData("AT+SAPBR=3,1,\"Contype\",\"GPRS\"",TIMEOUT_GSM, 1);
  if (response.equals("\r\nOK\r\n")){
    sendData("AT+SAPBR=1,1",TIMEOUT_GSM, 1);
    response = sendData("AT+SAPBR=2,1",TIMEOUT_GSM, 1);
      
    String tempStr = response.substring(0,14);
    if (tempStr.equals("\r\n+SAPBR: 1,1,")){
      displayMessage("GPRS Established", TEXT_SIZE);

      // Get the time
      displayMessage("Getting time via NTP...", TEXT_SIZE);
      response = sendData("AT+CNTPCID=1",TIMEOUT_GSM, 1);
      if (response.equals("\r\nOK\r\n")){
        response = sendData("AT+CNTP=\"pool.ntp.org\",22",TIMEOUT_GSM, 1);
        if (response.equals("\r\nOK\r\n")){
          response = sendData("AT+CNTP",TIMEOUT_GSM * 5, 1);
          if (response.equals("\r\nOK\r\n\r\n+CNTP: 1\r\n")){
            response = sendData("AT+CCLK?", TIMEOUT_GSM, 1);
            displayMessage("Time sync successful", TEXT_SIZE);
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
      if (millis() - startTime > TIMEOUT_SEN) 
        return 0; //Timeout error
    }

    if (Serial2.read() == 0x42) 
      break; // We have first part of header
  }
  
  while (1){
    while (!Serial2.available()){
      delay(1);
      if (millis() - startTime > TIMEOUT_SEN) 
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
      if (millis() - startTime > TIMEOUT_SEN) 
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
      if (millis() - startTime > TIMEOUT_SEN) 
        return 0; //Timeout error
    }

    if (Serial3.read() == 0x42) 
      break; // We have first part of header
  }
  
  while (1){
    while (!Serial3.available()){
      delay(1);
      if (millis() - startTime > TIMEOUT_SEN) 
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
      if (millis() - startTime > TIMEOUT_SEN) 
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

void displayMessage(String text, int text_size) {
  Serial.println(text);
  
  disp.setTextSize(text_size);
  disp.setTextColor(ILI9340_BLUE);
    
  if (textline < 25) {
    textline = textline + text_size;
  } else {
    disp.fillScreen(ILI9340_GREEN);
    disp.setCursor(0, 0);
    textline = 0;
  }
  
  disp.println(text);
}

