/*
  This project is to measure the tank level of three different liquid
  measuring PETROL, DIESEL and KEROSENE level using ultrasonic sensor HC-SR04
  Transfering the Data(s) into the cloud using GSM module using SIM800

*/

#include <util/delay.h>

#define PETRO_TRIG  5
#define PETRO_ECHO  6
#define TANK_HEIGHT 400   //in cm
  

volatile double petrolDuration, petrolDistance, petrolLevel;
  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("The Level readings are below :");
  delay(1500);
  
  // Note: verify the following code 
  pinMode(PETRO_TRIG, OUTPUT);     // Digital PIN 5
  pinMode(PETRO_ECHO, INPUT);      // Digital PIN 6
}

void loop() {
  // put your main code here, to run repeatedly:
  
  /// The following code cater for measuring petrol
  petrolLevel = distance_detect(PETRO_TRIG, PETRO_ECHO);
  
  Serial.print("The Level of the Water tank is: ");
  Serial.println(petrolLevel);
  _delay_ms(1000);

}

double distance_detect(uint8_t trig_pin, uint8_t echo_pin)
{
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  petrolDuration = pulseIn(echo_pin, HIGH);
  petrolDistance = static_cast<double>(petrolDuration)/58.2;
  return (TANK_HEIGHT - petrolDistance);
}

