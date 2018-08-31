// Libraries
#include "SPI.h"
#include "SD.h"
#include <LiquidCrystal.h>

// Pins for the GSM module
#define GSM_RX 19
#define GSM_TX 18
#define GSM_PWR 47

// Pins for load cell
#define SEN_CSB 6
#define SEN_SCLK 7
#define SEN_DOUT 2

// Pins for LCD display
#define LCD_VSS 13
#define LCD_VDD 12
#define LCD_CNTRST 11
#define LCD_RST 10
#define LCD_RW 9
#define LCD_ENB 8
#define LCD_D0 22
#define LCD_D1 24
#define LCD_D2 26
#define LCD_D3 28
#define LCD_D4 30
#define LCD_D5 32
#define LCD_D6 34
#define LCD_D7 36

int TOTAL_READING = 10;   // Number of averaged readings

int TIMEOUT_SEN = 1500;   // Timeout period for sensor reading
int TIMEOUT_GSM = 1000;   // Timeout period for GSM module

// Data from sensors
int sen_load = 0;

// Averaged data from sensors
int sen_load_avg = 0;
 
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

// Display
LiquidCrystal lcd(LCD_RST, LCD_ENB, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup() {
  // Pin modes
  pinMode(SEN_CSB, OUTPUT);
  pinMode(SEN_DOUT, INPUT);
  pinMode(SEN_SCLK, OUTPUT);
  pinMode(SEN_DOUT, INPUT_PULLUP);
  pinMode(LCD_RW, OUTPUT);
  pinMode(LCD_CNTRST, OUTPUT);
  pinMode(LCD_VDD, OUTPUT);
  pinMode(LCD_VSS, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  
  // Ports 
  digitalWrite(SEN_CSB,HIGH);
  digitalWrite(SEN_SCLK,HIGH);
  digitalWrite(LCD_RW, LOW);
  analogWrite(LCD_CNTRST, 75);
  digitalWrite(LCD_VDD, HIGH);
  digitalWrite(LCD_VSS, LOW);
  digitalWrite(LCD_RST, LOW);

  // Serial port to computer
  Serial.begin(9600);
  displayMessage("Initialzing...");  
  
  // Initializing SD card
  if (!SD.begin()){
    displayMessage("SD card initializing failed"); 
  }

  // Initialize display
  lcd.clear();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Init...");

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
  sen_load_avg = 0;
  
  displayMessage("Collecting data from sensors...");

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Loadcell : ");
  
  for (int i = 0; i < TOTAL_READING; i++) {
    sen_load = sensorDataRead();

    String sens_text = "EL";
    sens_text = sens_text + i + " :"+ sen_load + "     ";
    displayMessage(sens_text);
    lcd.setCursor(0, 0);
    lcd.print("TL : 0");
    lcd.setCursor(0, 1);
    lcd.print(sens_text);
  
    sen_load_avg = sen_load_avg + sen_load;

    delay(1000);
  }
  
  // Display Time
  while (!getTime()) {
    initializeGSM();
  }

  // If possible get GPS coordinate
  if (getGPS()){
    String gps_text = "GPS location - Lat ";
    gps_text = gps_text + latitude + ", Lon " + longtitude;
    displayMessage(gps_text);
  } else {
    displayMessage("GPS location failed");
  }
  
  sen_load_avg = sen_load_avg / TOTAL_READING;
  sen_load_avg = sen_load_avg * 1 + 0;              // change calibration here************************************************
  
  String text = "Load cell : ";
  text = text + "" + (sen_load_avg) + "\r\n Samples : " + TOTAL_READING;      
  displayMessage(text);
  
  // Sending data to server
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Send int...");
  
  displayMessage("Sending to server...");
  for (int i = 0; i < 3; i++) {
    if (sendDataToServer(sen_load_avg)){
      displayMessage("Data sent to server!!");
      lcd.print("OKAY");
      break;
    } else {
      displayMessage("Server connection failed, retrying...");
      delay(1000);
      initializeGSM();
    }
  }

  // Write to SD card
  lcd.setCursor(0,1);
  lcd.print("Store SD...");
  displayMessage("Writing to SD card...");
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    String dataString;
    dataString = "";
    dataString = dataString + "" + hour + ":" + minute + ":" + second + "T" + day + ":" + month + ":" + year;
    dataString = dataString + "  " + sen_load_avg + "\r\n";
    dataFile.println(dataString);
     
    dataFile.close();
    displayMessage("Writing complete!!");
    lcd.print("OKAY");
  } else {
    displayMessage("Error with the SD writing");
    lcd.print("FAIL");
  }
  
  delay(10000);
}

// Sending data to server
bool sendDataToServer(int senValue) {
  String response = "";

  // Initialize HTTP connection
  response = sendData("AT+HTTPINIT", TIMEOUT_GSM, 1);
  delay(1000);
  response = sendData("AT+HTTPPARA=\"CID\",1", TIMEOUT_GSM, 1);
  if (response.equals("AT+HTTPPARA=\"CID\",1\r\n\r\nOK\r\n")){
    // Set the URL
    String command = "AT+HTTPPARA=\"URL\",\"http://www.yung.lk/airquality/record.php?id=20000";
    command = command + "&battery=34&longitude=" + longtitude + "&latitude=" + latitude + "&data={ \\\"datapoints\\\": [ { \\\"time\\\": \\\"";
    command = command + hour + ":" + minute + ":" + second + "T" + day + ":" + month + ":" + year + "\\\", \\\"pm10\\\": ";
    command = command + senValue + ", \\\"pm25\\\" : 0 }]}\"";
    
    response = sendData(command, TIMEOUT_GSM, 1);
    String neededResponse = "AT+HTTPPARA=\"URL\",\"http://www.yung.lk/airquality/record.php?id=ime\\\": \\\"";
    neededResponse = neededResponse + hour + ":" + minute + ":" + second + "T" + day + ":" + month + ":" + year + "\\\", \\\"pm10\\\": " + senValue + ", \\\"pm25\\\" : 0 }]}\"\r\n\r\nOK\r\n";
    
    // Send data to server
    response = sendData("AT+HTTPACTION=0", 5*TIMEOUT_GSM, 1);
    String temp = response.substring(0,44);
    if (temp.equals("AT+HTTPACTION=0\r\n\r\nOK\r\n\r\n+HTTPACTION: 0,200,")) {
      response = sendData("AT+HTTPREAD", TIMEOUT_GSM, 1);
      if (response.equals("AT+HTTPREAD\r\n\r\n+HTTPREAD: 8\r\nOKAY<br>\r\nOK\r\n")) {
        return 1;
      }
    }
  }
  return 0;
}

// Initializes the GSM module
void initializeGSM(){
  String response = "";

  // GSM operation verification
  displayMessage("GSM Verifying...");
  
  while (1){
    lcd.setCursor(0,1);
    lcd.print("GSM...");
    // GSM echo mode off
    sendData("ATE0", TIMEOUT_GSM, 1);
    delay(1000);

    // POST on GSM module
    response = sendData("AT+CFUN?", TIMEOUT_GSM, 1);
    if (response.equals("AT+CFUN?\r\n\r\n+CFUN: 1\r\n\r\nOK\r\n")){
      lcd.print("A");
      // SIM functional verification
      response = sendData("AT+CPIN?", TIMEOUT_GSM, 1);
      if (response.equals("AT+CPIN?\r\n\r\n+CPIN: READY\r\n\r\nOK\r\n")){
        displayMessage("GSM module functional");
        lcd.print("B");
        
        // Register with the service provider
        displayMessage("Connecting...");
        for (int i = 0; i < 3; i++) {
          response = sendData("AT+CREG?", TIMEOUT_GSM, 1);
          if (response.equals("AT+CREG?\r\n\r\n+CREG: 0,1\r\n\r\nOK\r\n")){
            lcd.print("C");
            displayMessage("Connection established");
            
            // Establish GPRS connection and get the time
            displayMessage("GPRS connecting...");
            for (int j = 0; j < 3; j++) {
              if (connectGPRS()) {
                if (getTime()) {
                  lcd.print("D");
                  displayMessage("GSM functional!!");
                  delay(1000);
                  return; 
                } else {
                  displayMessage("Time syncing failed!! Retrying...");
                  delay(1000);
                }
              } else {
                displayMessage("Connect GPRS failed!! Retrying...");
                delay(1000);
              }
            }
          } else {
            displayMessage("Connection error!! Retrying...");
            delay(1000); 
          }
        }
      } else {
        displayMessage("GSM module SIM error!!");
        delay(1000);
      }
    } else {
      displayMessage("GSM module POST error!!");
      delay(1000);
    }
    displayMessage("GSM module error!! Retrying from beginning...");
    delay(1000); 
  }
}

bool getGPS(){
  displayMessage("Getting GPS location...");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GPS...");
    
  String response = sendData("AT+CGPSPWR=1", TIMEOUT_GSM, 1);
  if (response.equals("AT+CGPSPWR=1\r\n\r\nOK\r\n")){
    response = sendData("AT+CGPSSTATUS?", TIMEOUT_GSM, 1);
    lcd.print("A");
  
    if (response.equals("AT+CGPSSTATUS?\r\n+CGPSSTATUS: Location 3D Fix\r\n\r\nOK\r\n")){
      lcd.print("B");
      response = sendData("AT+CGPSINF=0", TIMEOUT_GSM, 1);

      // Parsing the response
      if (response.substring(14,28).equals("\r\n+CGPSINF: 0,")){
        longtitude = response.substring(28,38).toDouble();
        latitude = response.substring(39,50).toDouble();
        lcd.print("C");
        return true;
      } else {
        displayMessage("GPS location format error");
      }
    } else {
      displayMessage("GPS location still unfixed");
    }
  } else {
    displayMessage("GPS module power-on failure");
  }
  lcd.print("FAIL");
  return false;
}

bool getTime(){
  displayMessage("Getting time...");
  
  String response = sendData("AT+CCLK?", TIMEOUT_GSM, 1);

  if (response.substring(10,19).equals("\r\n+CCLK: ")){
    year   = response.substring(20, 22).toInt();
    month  = response.substring(23, 25).toInt();
    day    = response.substring(26, 28).toInt();
    hour   = response.substring(29, 31).toInt();
    minute = response.substring(32, 34).toInt();
    second = response.substring(35, 37).toInt();
    
    String text = "";
    text = text + "Time " + hour + ":" + minute + ":" + second + " on " + year + "/" + month + "/" + day;
    displayMessage(text);
    return true;
  } else {
    displayMessage("Time unavailable");
    return false;
  }
  
}

bool connectGPRS(){
  String response = sendData("AT+SAPBR=3,1,\"Contype\",\"GPRS\"",TIMEOUT_GSM, 1);
  if (response.equals("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n\r\nOK\r\n")){
    sendData("AT+SAPBR=1,1",TIMEOUT_GSM, 1);
    response = sendData("AT+SAPBR=2,1",TIMEOUT_GSM, 1);
      
    String tempStr = response.substring(14,28);
    if (tempStr.equals("\r\n+SAPBR: 1,1,")){
      displayMessage("GPRS Established");

      // Get the time
      displayMessage("Getting time via NTP...");
      response = sendData("AT+CNTPCID=1",TIMEOUT_GSM, 1);
      if (response.equals("AT+CNTPCID=1\r\n\r\nOK\r\n")){
        response = sendData("AT+CNTP=\"pool.ntp.org\",22",TIMEOUT_GSM, 1);
        if (response.equals("AT+CNTP=\"pool.ntp.org\",22\r\n\r\nOK\r\n")){
          response = sendData("AT+CNTP",TIMEOUT_GSM * 5, 1);
          if (response.equals("AT+CNTP\r\n\r\nOK\r\n\r\n+CNTP: 1\r\n")){
            response = sendData("AT+CCLK?", TIMEOUT_GSM, 1);
            displayMessage("Time sync successful");
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
    Serial.println("**************\r\n***Sent:*******\r\n" + command + "\r\n***Received:***" + response + "\r\n**************");
  }    
  return response;
}

// Return ADC value
int sensorDataRead(void){
  int value,temp,count;
  temp=0;
  count=0;
  value=0;

  while(count<16){
    digitalWrite(SEN_CSB,0);
    delayMicroseconds(1);
    digitalWrite(SEN_SCLK,0);
    delayMicroseconds(1);
    if(digitalRead(SEN_DOUT)==1){
      temp=1;
    }else{
      temp=0;
    }
    value = (value << 1) + temp ;
    delayMicroseconds(1);
    digitalWrite(SEN_SCLK,1);
    delayMicroseconds(1);
    count++;
  }
  digitalWrite(SEN_CSB,1);

  if(value > 8191){
    value = value - 16384;
  }
  
  return value; 
}

void displayMessage(String text) {
  Serial.println(text);
}

