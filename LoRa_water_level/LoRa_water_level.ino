#include <SPI.h>
#include "ArduinoLowPower.h"
#include <LoRa.h>
#define trigPin 2 //Sensor Trip pin connected to Arduino pin 2
#define echoPin 3 //Sensor Echo pin connected to Arduino pin 3
int counter = 0;
float pingTime;//time for ping to travel from sensor to target and return
float h = 0;
float v = 0; // volume of the water
float speedOfSound = 776.5; //Speed of sound in miles per hour when temp is 77 degrees fahrenheit.

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.println("Water level sensor");
  if (!LoRa.begin(865E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  digitalWrite(trigPin, LOW); //Set trigger pin low
  delayMicroseconds(2000); //Let signal settle
  digitalWrite(trigPin, HIGH); //Set trigPin high
  delayMicroseconds(15); //Delay in high state
  digitalWrite(trigPin, LOW); //ping has now been sent
  delayMicroseconds(10); //Delay in high state


  pingTime = pulseIn(echoPin, HIGH);
  pingTime = pingTime / 1000000; //convert pingTime to seconds by dividing by 1000000 (microseconds in a second)
  pingTime = pingTime / 3600; //convert pingtime to hours by dividing by 3600 (seconds in an hour)
  h = speedOfSound * pingTime; //This will be in miles, since speed of sound was miles per hour
  h = h / 2; //Remember ping travels to target and back from target, so you must divide by 2 for actual target distance.
  h = h * 160934.4; //Convert miles to cms by multipling by 160934.4 (cms per mile)
  Serial.print("height=");
  Serial.println(h);


  v = 22 / 7 * 53 * 53 * (115 - h) / 1000; //Volume of cylinder formula, here r is 53cm
  float vp = v * 100 / 1000; //1000L is the tank volume
  Serial.println(vp);
  Serial.println("%");
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello, the remaining water in the tank is ");
  LoRa.print(v);
  LoRa.print("Ltrs");
  LoRa.endPacket();
  delay(5000);
  //LowPower.sleep(5000);
  counter++;
}
