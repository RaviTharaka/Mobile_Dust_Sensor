#define ENB_PIN 2
#define RESET_PIN 3
#define RX1 19
#define TX1 18

int pm25 = 0;
int pm10 = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  Serial1.begin(9600);

  //Set and reset
  pinMode(ENB_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(ENB_PIN,HIGH);
  digitalWrite(RESET_PIN,HIGH);
  
}

void loop() {
  if (dataAvailable()){
    Serial.print("Particle Matter [2.5]:");
    Serial.print(pm25, 1);
    Serial.print("ug/m3 [10]:");
    Serial.print(pm10, 1);
    Serial.print("ug/m3");
    Serial.println();
  } else {
    Serial.println("Timeout or CRC error");
    Serial.println("Double check blue wire goes to pin 10");
  }

  delay(5);
}

boolean dataAvailable(void){
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
  pm25 = ((float)sensorValue[10] * 256 + sensorValue[11]);
  pm10 = ((float)sensorValue[6] * 256 + sensorValue[7]);

  return (true); //We've got a good reading!
  
}
