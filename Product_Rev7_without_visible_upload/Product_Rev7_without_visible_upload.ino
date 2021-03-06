// Libraries
#include "SPI.h"
#include "SD.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

// Pins for the GSM module
#define GSM_RX 19
#define GSM_TX 18
#define GSM_PWR 47

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
#define DISP_SCLK 12
#define DISP_MISO 13
#define DISP_MOSI 11
#define DISP_CS 8
#define DISP_DC 10
#define DISP_RST 9

int TOTAL_READING = 10;   // Number of averaged readings
int CORR_READING = 5;     // Number of valid readings required

int TIMEOUT_SEN = 1500;   // Timeout period for sensor reading
int TIMEOUT_GSM = 1000;   // Timeout period for GSM module
int TEXT_SIZE = 1;        // Display text size

int SEN_A_ID = 10005;     // Unique ID for sensor A
int SEN_B_ID = 10010;     // Unique ID for sensor B

// Sensor types
int sen1 = 21;
int sen2 = 0;

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

// GPS Location
double latitude = 0;
double longtitude = 0;

// Initialize display
Adafruit_ILI9340 disp = Adafruit_ILI9340(DISP_CS, DISP_DC, DISP_MOSI, DISP_SCLK, DISP_RST, DISP_MISO);
int textline = 0;

void setup() {
  // Pin modes
  pinMode(SEN1_ENB, OUTPUT);
  pinMode(SEN1_RST, OUTPUT);
  pinMode(SEN2_ENB, OUTPUT);
  pinMode(SEN2_RST, OUTPUT);
  pinMode(GSM_PWR, OUTPUT);
  
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
  displayMessage("Initialzing...", TEXT_SIZE, 1);  

  // Serial port to sensors 
  Serial2.begin(9600);
  Serial3.begin(9600);

  // Initializing SD card
  if (!SD.begin()){
    displayMessage("SD card initializing failed", TEXT_SIZE, 1); 
  }

  // Initializing the GSM module
  Serial1.begin(9600);
  delay(1000);
  digitalWrite(GSM_PWR,HIGH);
  delay(3000);
  digitalWrite(GSM_PWR,LOW);
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

  disp.fillScreen(ILI9340_BLACK);
  disp.setCursor(0, 0);
  textline = 0;
  
  displayMessage("Collecting data from sensors...", TEXT_SIZE, 1);
  
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
  for (int i = 0; i < 2; i = i + 1) {
    if(getTime()){
      break;
    }
    initializeGSM();
  }
  
  if (sen2_corr > CORR_READING) {
    sen2_pm25_avg = sen2_pm25_avg / sen2_corr;
    sen2_pm10_avg = sen2_pm10_avg / sen2_corr;
  }
  
  if (sen1_corr > CORR_READING) {
    sen1_pm25_avg = sen1_pm25_avg / sen1_corr;
    sen1_pm10_avg = sen1_pm10_avg / sen1_corr;
    
    String text = "Sensor -";
    text = text + " " + sen1 +"\r\n PM2.5   : " + ((sen1_pm25_avg * 0.4189) + 3.9901) + "\r\n PM10    : " + ((sen1_pm10_avg * 0.5328) + 14.082) + "\r\n Samples : " + sen1_corr;
    displayMessage(text, 3, 1);
  } else {
    String text = "Sensor -";
    text = text + " " + sen1 + " : Only " + sen1_corr + " data is correct";
    displayMessage(text, 2, 1);
    text = "Log : ";
    for (int i = 0; i < TOTAL_READING; i++) {
      text = text + sen1_status[i]; 
    }
    displayMessage(text, 2, 1);
    
    if (sen2_corr > CORR_READING) {
      text = "Sensor -";
      text = text + " " + sen2 + "\r\n PM2.5   : " + (sen2_pm25_avg * 0.4189 + 3.9901) + "\r\n PM10    : " + (sen2_pm10_avg * 0.5328 + 14.082) + "\r\n Samples : " + sen2_corr;
      //displayMessage(text , 3);
    } else {
      String text = "Sensor -";
      text = text + " " + sen2 + " : Only" + sen2_corr + " data is correct";
      //displayMessage(text , 2);
    }
  }

  // Sending data to server
  if (sen1_corr > CORR_READING) {
    displayMessage("Sensor 1 sending to server...", TEXT_SIZE, 0);
    for (int i = 0; i < 1; i++) {
      if (sendDataToServer(SEN_A_ID ,sen1_pm10_avg, sen1_pm25_avg)){
        displayMessage("Data 1 sent to server!!", TEXT_SIZE, 0);
        break;
      } else {
        displayMessage("Server connection failed, retrying...", TEXT_SIZE, 0);
        delay(1000);
        initializeGSM();
      }
    }
  }

  if (sen2_corr > CORR_READING) {
    displayMessage("Sensor 2 sending to server...", TEXT_SIZE, 0);
    for (int i = 0; i < 1; i++) {
      if (sendDataToServer(SEN_B_ID ,sen2_pm10_avg, sen2_pm25_avg)){
        displayMessage("Data 2 sent to server!!", TEXT_SIZE, 0);
        break;
      } else {
        displayMessage("Server connection failed, retrying...", TEXT_SIZE, 0);
        delay(1000);
        initializeGSM();
      }
    }
  }

  // Write to SD card
  displayMessage("Writing to SD card...", TEXT_SIZE, 1);
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    String dataString;
    if (sen1 != 0) {
      dataString = "";
      dataString = dataString + SEN_A_ID + " " + hour + ":" + minute + ":" + second + "T" + day + ":" + month + ":" + year;
      dataString = dataString + " PM10: " + sen1_pm10_avg + " PM2.5: " + sen1_pm25_avg + "\r\n";
      dataFile.println(dataString);
    }

    if (sen2 != 0) {
      dataString = "";
      dataString = dataString + SEN_B_ID + " " + hour + ":" + minute + ":" + second + "T" + day + ":" + month + ":" + year;
      dataString = dataString + " PM10: " + sen2_pm10_avg + " PM2.5: " + sen2_pm25_avg + "\r\n";
      dataFile.println(dataString);
    }
     
    dataFile.close();
    displayMessage("Writing complete!!", TEXT_SIZE, 1);
  } else {
    displayMessage("Error with the SD writing", TEXT_SIZE, 1);
  }
  
  delay(10000);
  
   
}

// Sending data to server
bool sendDataToServer(int index, int pm10, int pm25) {
  String response = "";

  // Initialize HTTP connection
  response = sendData("AT+HTTPINIT", TIMEOUT_GSM, 1);
  delay(1000);
  response = sendData("AT+HTTPPARA=\"CID\",1", TIMEOUT_GSM, 1);
  if (response.equals("\r\nOK\r\n")){
    // Set the URL
    String command = "AT+HTTPPARA=\"URL\",\"http://www.yung.lk/airquality/record.php?id=";
    command = command + index + "&battery=34&longitude="+ longtitude +"&latitude=" + latitude + "&data={ \\\"datapoints\\\": [ { \\\"time\\\": \\\"";
    command = command + hour + ":" + minute + ":" + second + "T" + day + ":" + month + ":" + year + "\\\", \\\"pm10\\\": ";
    command = command + pm10 + ", \\\"pm25\\\" : " + pm25 +" }]}\"";
    
    response = sendData(command, TIMEOUT_GSM, 1);
    
    if (response.equals("\r\nOK\r\n")){
      // Send data to server
      response = sendData("AT+HTTPACTION=0", 5*TIMEOUT_GSM, 1);
      String temp = response.substring(0,27);
      if (temp.equals("\r\nOK\r\n\r\n+HTTPACTION: 0,200,")) {
        response = sendData("AT+HTTPREAD", TIMEOUT_GSM, 1);
        if (response.equals("\r\n+HTTPREAD: 8\r\nOKAY<br>\r\nOK\r\n")) {
          displayMessage("URL accepted", TEXT_SIZE, 0);
          return 1;
        }
      }
    }
  }
  return 0;
}

// Initializes the GSM module
void initializeGSM(){
  //disp.fillScreen(ILI9340_BLACK);
  //disp.setCursor(0, 0);
  //textline = 0;
  
  String response = "";

  // GSM operation verification
  displayMessage("GSM Verifying...", TEXT_SIZE, 0);
  
  // GSM echo mode off
  sendData("ATE0", TIMEOUT_GSM, 1);
  delay(1000);

  // POST on GSM module
  response = sendData("AT+CFUN?", TIMEOUT_GSM, 1);
  if (response.equals("\r\n+CFUN: 1\r\n\r\nOK\r\n")){
    // SIM functional verification
    response = sendData("AT+CPIN?", TIMEOUT_GSM, 1);
    if (response.equals("\r\n+CPIN: READY\r\n\r\nOK\r\n")){
      displayMessage("GSM module functional", TEXT_SIZE, 0);
      
      // Register with the service provider
      displayMessage("Connecting...", TEXT_SIZE, 0);
      for (int i = 0; i < 1; i++) {
        response = sendData("AT+CREG?", TIMEOUT_GSM, 1);
        if (response.equals("\r\n+CREG: 0,1\r\n\r\nOK\r\n")){
          displayMessage("Connection established", TEXT_SIZE, 0);
          
          // Establish GPRS connection and get the time
          displayMessage("GPRS connecting...", TEXT_SIZE, 0);
          for (int j = 0; j < 1; j++) {
            if (connectGPRS()) {
              if (getTime()) {
                displayMessage("GSM functional!!", TEXT_SIZE, 0);
                delay(1000);

                // Refreshing display
                disp.fillScreen(ILI9340_GREEN);
                disp.setCursor(0, 0);
                textline = 0; 
                return; 
              } else {
                displayMessage("Time syncing failed!! Retrying...", TEXT_SIZE, 0);
                delay(1000);
              }
            } else {
              displayMessage("Connect GPRS failed!! Retrying...", TEXT_SIZE, 0);
              delay(1000);
            }
          }
        } else {
          displayMessage("Connection error!! Retrying...", TEXT_SIZE, 0);
          delay(1000); 
        }
      }
    } else {
      displayMessage("GSM module SIM error!!", TEXT_SIZE, 0);
      delay(1000);
    }
  } else {
    displayMessage("GSM module POST error!!", TEXT_SIZE, 0);
    delay(1000);
  }
  displayMessage("GSM module error!! Retrying from beginning...", TEXT_SIZE, 0);
  
  delay(1000); 

}

bool getGPS(){
  displayMessage("Getting GPS location...", TEXT_SIZE, 1);
  
  String response = sendData("AT+CGPSPWR=1", TIMEOUT_GSM, 1);
  if (response.equals("\r\nOK\r\n")){
    response = sendData("AT+CGPSSTATUS?", TIMEOUT_GSM, 1);
    if (response.equals("\r\n+CGPSSTATUS: Location 3D Fix\r\n\r\nOK\r\n")){
      response = sendData("AT+CGPSINF=0", TIMEOUT_GSM, 1);

      // Parsing the response
      if (response.substring(0,14).equals("\r\n+CGPSINF: 0,")){
        longtitude = response.substring(14,24).toDouble();
        latitude = response.substring(25,36).toDouble();
        return true;
      } else {
        displayMessage("GPS location format error", TEXT_SIZE, 1);
      }
    } else {
      displayMessage("GPS location still unfixed", TEXT_SIZE, 1);
    }
  } else {
    displayMessage("GPS module power-on failure", TEXT_SIZE, 1);
  }

  return false;
}

bool getTime(){
  displayMessage("Getting time...", TEXT_SIZE, 0);
  
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
    displayMessage(text, TEXT_SIZE, 0);
    return true;
  } else {
    displayMessage("Time unavailable", TEXT_SIZE, 0);
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
      displayMessage("GPRS Established", TEXT_SIZE, 0);

      // Get the time
      displayMessage("Getting time via NTP...", TEXT_SIZE, 0);
      response = sendData("AT+CNTPCID=1",TIMEOUT_GSM, 1);
      if (response.equals("\r\nOK\r\n")){
        response = sendData("AT+CNTP=\"pool.ntp.org\",22",TIMEOUT_GSM, 1);
        if (response.equals("\r\nOK\r\n")){
          response = sendData("AT+CNTP",TIMEOUT_GSM * 5, 1);
          if (response.equals("\r\nOK\r\n\r\n+CNTP: 1\r\n")){
            response = sendData("AT+CCLK?", TIMEOUT_GSM, 1);
            //displayMessage("Time sync successful", TEXT_SIZE);
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
    Serial.println("**************\r\n***Sent:***\r\n" + command + "\r\n***Received:***" + response + "\r\n**************");
  }    
  return response;
}

// Return 0 = Timeout, 1 = CRC Error, 2 = Success, 3 = No sensor
int sensorDataAvailable_1(void){
  if (sen1 == 21) {
    //Spin until we hear meassage header byte
    long startTime = millis();
  
    while (1)
    {
      while (!Serial2.available())
      {
        delay(1);
        if (millis() - startTime > 1500) 
        return 0; //Timeout error
      }
  
      if (Serial2.read() == 0xAA) break; //We have the message header
    }
    
    //Read the next 9 bytes
    byte sensorValue[10];
    for (byte spot = 1 ; spot < 10 ; spot++)
    {
      startTime = millis();
      while (!Serial2.available())
      {
        delay(1);
        if (millis() - startTime > 1500) return (false); //Timeout error
      }
  
      sensorValue[spot] = Serial2.read();
    }

    //Check CRC
    byte crc = 0;
    for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
      crc += sensorValue[x];
    if (crc != sensorValue[8])
      return 1; //CRC error
  
    //Update the global variables
    sen1_pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
    sen1_pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;
  
    return 2;
    
  } else if (sen1 == 7003 | sen1 == 5003) {
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
    if (sen1 == 5003){
      sen1_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
      sen1_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
    } else if (sen1 == 7003) {
      sen1_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
      sen1_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
    }
    return 2; // We've got a good reading!
  } else {
    return 3; // Sensor is unconnected
  }
}


// Return 0 = Timeout, 1 = CRC Error
int sensorDataAvailable_2(void){
  if (sen2 == 21) {
    //Spin until we hear meassage header byte
    long startTime = millis();
  
    while (1)
    {
      while (!Serial3.available())
      {
        delay(1);
        if (millis() - startTime > 1500) 
        return 0; //Timeout error
      }
  
      if (Serial3.read() == 0xAA) break; //We have the message header
    }
    
    //Read the next 9 bytes
    byte sensorValue[10];
    for (byte spot = 1 ; spot < 10 ; spot++)
    {
      startTime = millis();
      while (!Serial3.available())
      {
        delay(1);
        if (millis() - startTime > 1500) return (false); //Timeout error
      }
  
      sensorValue[spot] = Serial3.read();
    }

    //Check CRC
    byte crc = 0;
    for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
      crc += sensorValue[x];
    if (crc != sensorValue[8])
      return 1; //CRC error
  
    //Update the global variables
    sen2_pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
    sen2_pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;
  
    return 2;
    
  } else if (sen2 == 5003 | sen2 == 7003) {
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
    if (sen2 == 5003){
      sen2_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
      sen2_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
    } else if (sen2 == 7003) {
      sen2_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
      sen2_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
    }
    return 2; // We've got a good reading!
  } else {
    return 3; // Sensor is unconnected
  }
}

void displayMessage(String text, int text_size, int send_to_display) {
  Serial.println(text);

  if (send_to_display) {
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
}

