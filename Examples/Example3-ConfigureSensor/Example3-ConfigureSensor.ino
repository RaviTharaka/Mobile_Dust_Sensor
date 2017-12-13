/*
  Reading bytes from SDS021 Air Quality PM2.5/10 Particle Sensor
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 8th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example show how to configure the sensor's options.

  Press f to read the firmware version
  Press s to put unit to sleep
  Press w to wake unit

  Hardware Connections:
    5V(on Arduino) to Red(on SDS021)
    GND to Black
    10 to Blue (TX from sensor)
    11 to Yellow (RX into sensor)

  Then open the Serial terminal at 9600bps.
*/

#include <SoftwareSerial.h>

SoftwareSerial particleSensor(10, 11); // RX, TX

float pm25; //2.5um particles detected in ug/m3
float pm10; //10um particles detected in ug/m3

unsigned int deviceID; //Two byte unique ID set by factor

void setup()
{
  Serial.begin(9600);
  Serial.println("Particle Sensor Example");

  particleSensor.begin(9600); //SDS021 reports at 1Hz at 9600bps
}

void loop()
{
  if (dataAvailable())
  {
    Serial.print("Particle Matter [2.5]:");
    Serial.print(pm25, 1);
    Serial.print("ug/m3 [10]:");
    Serial.print(pm10, 1);
    Serial.print("ug/m3");
    Serial.print(" DeviceID:0x");
    Serial.print(deviceID, HEX);
    Serial.println();
  }
  else
  {
    Serial.println("Timeout or CRC error");
    Serial.println("Double check blue wire goes to pin 10");
  }

  if (Serial.available())
  {
    printMenu();

    byte incoming = Serial.read();
    if (incoming == 'f')
    {
      getFirmwareVersion(); //Query firmware version
    }
    else if (incoming == 'w')
    {
      wakeUp();
    }
    else if (incoming == 's')
    {
      goToSleep();
    }
    else
    {
      printMenu();
    }
  }

  delay(5);

}

void printMenu()
{
  Serial.println();
  Serial.println("Available Commands: ");
  Serial.println("f) Get firmware version");
  Serial.println("w) wakeup");
  Serial.println("s) sleep!");
}

//Print the firmware version
void getFirmwareVersion(void)
{
  sendCommand(7, 0, 0); //Command number is 7, no databytes
}

//Tell the module to go to sleep
void goToSleep(void)
{
  sendCommand(6, 1, 0); //Command number is 6, set mode = 1, sleep = 0
}

//Tell module to start working!
void wakeUp(void)
{
  sendCommand(6, 1, 1); //Command number is 6, set mode = 1, work = 1
}

//Scans for incoming packet
//Times out after 1500 miliseconds
boolean dataAvailable(void)
{
  //Spin until we hear meassage header byte
  long startTime = millis();

  while (1)
  {
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    if (particleSensor.read() == 0xAA) break; //We have the message header
  }

  //Read the next 9 bytes
  byte sensorValue[10];
  for (byte spot = 1 ; spot < 10 ; spot++)
  {
    startTime = millis();
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    sensorValue[spot] = particleSensor.read();
  }

  //Check CRC
  byte crc = 0;
  for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
    crc += sensorValue[x];
  if (crc != sensorValue[8])
    return (false); //CRC error

  if (sensorValue[1] == 0xC0) //This is just a normal reading
  {
    //Update the global variables
    pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
    pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;

    deviceID = sensorValue[6] * 256 + sensorValue[7];
  }
  else if (sensorValue[1] == 0xC5) //Response to command
  {
    Serial.println();
    Serial.println("Response to command found");

    if (sensorValue[2] == 7) //Firmware response
    {
      Serial.print("Firmware version Y/M/D: ");
      Serial.print(sensorValue[3]);
      Serial.print("/");
      Serial.print(sensorValue[4]);
      Serial.print("/");
      Serial.print(sensorValue[5]);
      Serial.println();
    }
    else if (sensorValue[2] == 6) //Query/Set work and sleep modes
    {
      if (sensorValue[3] == 1) //Response to set mode
      {
        Serial.print("Sensor is going to ");
        if (sensorValue[4] == 0) Serial.println("sleep");
        else if (sensorValue[4] == 1) Serial.println("work");
      }
    }

    Serial.println();
  }

  Serial.print("Raw data:");
  for (int x = 1 ; x < 10 ; x++)
  {
    Serial.print(" ");
    Serial.print(x);
    Serial.print(":0x");
    Serial.print(sensorValue[x], HEX);
  }
  Serial.println();

  return (true); //We've got a good reading!
}

//Send a command packet to the module
//Requires the command number and two setting bytes
//Calculates CRC and attaches all header/ender bytes
//Assumes you are only talking to one sensor
void sendCommand(byte commandNumber, byte dataByte2, byte dataByte3)
{
  byte packet[19]; //It's 19 bytes big
  packet[0] = 0xAA; //Message header
  packet[1] = 0xB4; //Packet type = Command
  packet[2] = commandNumber; //Type of command we want to do
  packet[3] = dataByte2; //These are specific to each command
  packet[4] = dataByte3;

  for (byte x = 5; x < 15 ; x++)
    packet[x] = 0; //Reserved bytes

  packet[15] = 0xFF; //Talk to whatever sensor we are connected to. No specific device ID.
  packet[16] = 0xFF; //Talk to whatever sensor we are connected to. No specific device ID.

  //packet[15] = 0xA4; //Talk to specific sensor
  //packet[16] = 0xE6; //Talk to specific sensor

  //Caculate CRC
  byte crc = 0;
  for (byte x = 2 ; x < 17 ; x++)
    crc += packet[x];

  packet[17] = crc;
  packet[18] = 0xAB; //Tail

  //Display the contents of the command packet for debugging
  /*Serial.print("Command packet:");
    for(int x = 0 ; x < 19 ; x++)
    {
    Serial.print(" ");
    Serial.print(x);
    Serial.print(":0x");
    Serial.print(packet[x], HEX);
    }
    Serial.println();*/

  //The sensor seems to fail to respond to the first 2 or 3 times we send a command
  //Hardware serial doesn't have this issue but software serial does.
  //Sending 10 throw away characters at it gets the units talking correctly
  for (byte x = 0 ; x < 10 ; x++)
    particleSensor.write('!'); //Just get the software serial working

  //Send command packet
  for (byte x = 0 ; x < 19 ; x++)
    particleSensor.write(packet[x]);

  //Now look for response
  dataAvailable();
}

