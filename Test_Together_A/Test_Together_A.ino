#include <LiquidCrystal.h>

// Pins for the LCD display
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

// Pins for sensor A
#define SEN1_ENB 20
#define SEN1_RST 21
#define SEN1_RX 19
#define SEN1_TX 18

// Pins for sensor B
#define SEN2_ENB 15
#define SEN2_RST 14
#define SEN2_RX 17
#define SEN2_TX 16

// Sensor types
int sen1 = 5003;
int sen2 = 7003;

// Data from sensors
int sen1_pm25 = 0;
int sen1_pm10 = 0;
int sen2_pm25 = 0;
int sen2_pm10 = 0;

LiquidCrystal lcd(LCD_RST, LCD_ENB, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup() {
  // Pin modes
  pinMode(LCD_RW, OUTPUT);
  pinMode(LCD_CNTRST, OUTPUT);
  pinMode(LCD_VDD, OUTPUT);
  pinMode(LCD_VSS, OUTPUT);

  pinMode(SEN1_ENB, OUTPUT);
  pinMode(SEN1_RST, OUTPUT);
  pinMode(SEN2_ENB, OUTPUT);
  pinMode(SEN2_RST, OUTPUT);
  
  // Ports 
  digitalWrite(LCD_RW, LOW);
  analogWrite(LCD_CNTRST, 128);
  digitalWrite(LCD_VDD, HIGH);
  digitalWrite(LCD_VSS, LOW);

  digitalWrite(SEN1_ENB,HIGH);
  digitalWrite(SEN1_RST,HIGH);
  digitalWrite(SEN2_ENB,HIGH);
  digitalWrite(SEN2_RST,HIGH);

  // Serial port to computer
  Serial.begin(9600);
  Serial.println("Initializing...");

  // Serial port to sensors 
  Serial1.begin(9600);
  Serial2.begin(9600);
  
  // Port to LCD
  lcd.begin(16, 2);
  lcd.print("Initializing...");
  delay(2000);
}

void loop() {
  lcd.clear();
  
  if (dataAvailable_1()){
    sen1_pm25 = sen1_pm25 * 1;
    sen1_pm10 = sen1_pm10 * 1;
    
    Serial.print("A 2.5:");
    Serial.print(sen1_pm25, 1);
    Serial.print(" 10:");
    Serial.print(sen1_pm10, 1);
    Serial.println();

    lcd.setCursor(0, 0);
    lcd.print("A 2.5:");
    lcd.setCursor(6, 0);
    lcd.print(sen1_pm25);
    lcd.setCursor(8, 0);
    lcd.print("   10:");
    lcd.setCursor(14, 0);
    lcd.print(sen1_pm10);
  } else {
    Serial.println("Timeout");
    lcd.setCursor(0, 0);
    lcd.print("Sen A Timeout!!");
  }

  delay(5);

  if (dataAvailable_2()){
    sen2_pm25 = sen2_pm25 * 1;
    sen2_pm10 = sen2_pm10 * 1;
    
    Serial.print("B 2.5:");
    Serial.print(sen2_pm25, 1);
    Serial.print(" 10:");
    Serial.print(sen2_pm10, 1);
    Serial.println();

    lcd.setCursor(0, 1);
    lcd.print("B 2.5:");
    lcd.setCursor(6, 1);
    lcd.print(sen2_pm25);
    lcd.setCursor(8, 1);
    lcd.print("   10:");
    lcd.setCursor(14, 1);
    lcd.print(sen2_pm10);
  } else {
    Serial.println("Timeout");
    lcd.setCursor(0, 1);
    lcd.print("Sen B Timeout!!");
  }

  delay(1000);

}


boolean dataAvailable_1(void){
  //Spin until we hear meassage header byte
  long startTime = millis();

  while (1){
    while (!Serial1.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return (false); //Timeout error
    }

    if (Serial1.read() == 0x42) 
      break;
  }
  
  while (1){
    while (!Serial1.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return (false); //Timeout error
    }

    if (Serial1.read() == 0x4d) 
      break; //We have the complete message header
  }

  //Read the next 9 bytes
  byte sensorValue[30];
  for (byte spot = 0 ; spot < 30 ; spot++){
    startTime = millis();
    while (!Serial1.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return (false); //Timeout error
    }
    sensorValue[spot] = Serial1.read();
    
  }

  /*
  //Check CRC
  byte crc = 0;
  for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
    crc += sensorValue[x];
  
  if (crc != sensorValue[8])
    return (false); //CRC error
  */
  
  //Update the global variables
  if (sen1 = 5003){
    sen1_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen1_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  } else if (sen1 = 7003) {
    sen1_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen1_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  }
  return (true); //We've got a good reading!
  
}


boolean dataAvailable_2(void){
  //Spin until we hear meassage header byte
  long startTime = millis();

  while (1){
    while (!Serial2.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return (false); //Timeout error
    }

    if (Serial2.read() == 0x42) 
      break;
  }
  
  while (1){
    while (!Serial2.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return (false); //Timeout error
    }

    if (Serial2.read() == 0x4d) 
      break; //We have the complete message header
  }

  //Read the next 9 bytes
  byte sensorValue[30];
  for (byte spot = 0 ; spot < 30 ; spot++){
    startTime = millis();
    while (!Serial2.available()){
      delay(1);
      if (millis() - startTime > 1500) 
        return (false); //Timeout error
    }
    sensorValue[spot] = Serial2.read();
    
  }

  /*
  //Check CRC
  byte crc = 0;
  for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
    crc += sensorValue[x];
  
  if (crc != sensorValue[8])
    return (false); //CRC error
  */
  
  //Update the global variables
  if (sen2 = 5003){
    sen2_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen2_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  } else if (sen2 = 7003) {
    sen2_pm25 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    sen2_pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  }
  return (true); //We've got a good reading!
  
}
