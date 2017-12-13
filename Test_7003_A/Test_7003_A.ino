#define ENB_PIN1 2
#define RESET_PIN1 3
#define RX1 19
#define TX1 18

#define ENB_PIN2 4
#define RESET_PIN2 5
#define RX2 17
#define TX2 16


int sensor_1 = 5003;
int sensor_2 = 7003;

int pm25_1 = 0;
int pm10_1 = 0;

int pm25_2 = 0;
int pm10_2 = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  Serial1.begin(9600);
  Serial2.begin(9600);
  
  //Set and reset
  pinMode(ENB_PIN1, OUTPUT);
  pinMode(RESET_PIN1, OUTPUT);
  
  pinMode(ENB_PIN2, OUTPUT);
  pinMode(RESET_PIN2, OUTPUT);
  
  digitalWrite(ENB_PIN1,HIGH);
  digitalWrite(RESET_PIN1,HIGH);

  digitalWrite(ENB_PIN2,HIGH);
  digitalWrite(RESET_PIN2,HIGH);
  
}

void loop() {
  if (dataAvailable_1()){
    Serial.print("Particle Matter Sensor A [2.5]:");
    Serial.print(pm25_1, 1);
    Serial.print("ug/m3 [10]:");
    Serial.print(pm10_1, 1);
    Serial.print("ug/m3");
    Serial.println();
  } else {
    Serial.println("Timeout or CRC error");
    Serial.println("Double check blue wire goes to pin 10");
  }

  delay(5);

  if (dataAvailable_2()){
    Serial.print("Particle Matter Sensor B [2.5]:");
    Serial.print(pm25_2, 1);
    Serial.print("ug/m3 [10]:");
    Serial.print(pm10_2, 1);
    Serial.print("ug/m3");
    Serial.println();
  } else {
    Serial.println("Timeout or CRC error");
    Serial.println("Double check blue wire goes to pin 10");
  }
  
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
  if (sensor_1 = 5003){
    pm25_1 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    pm10_1 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  } else if (sensor_1 = 7003) {
    pm25_1 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    pm10_1 = ((float)sensorValue[6] * 256 + sensorValue[7]);
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
  if (sensor_2 = 5003){
    pm25_2 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    pm10_2 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  } else if (sensor_2 = 7003) {
    pm25_2 = ((float)sensorValue[4] * 256 + sensorValue[5]);
    pm10_2 = ((float)sensorValue[6] * 256 + sensorValue[7]);
  }
  return (true); //We've got a good reading!
  
}
