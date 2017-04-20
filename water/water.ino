/*
  This project is to measure the tank level of three different liquid
  measuring PETROL, DIESEL and KEROSENE level using ultrasonic sensor HC-SR04
  Transfering the Data(s) into the cloud using GSM module using SIM800

*/

#include <util/delay.h>

#define DIESEL_TRIG  2
#define DIESEL_ECHO  3

#define PETRO_TRIG   4
#define PETRO_ECHO   5

#define KEROS_TRIG   6
#define KEROS_ECHO   7

#define CONNECT_WIFI_LED 12
#define CONNECT_HOST_LED 13

// It is possible we dont use equal tank
#define DIESEL_TANK_HEIGHT  400.00   //in cm
#define KEROS_TANK_HEIGHT   300.00   //in cm
#define PETROL_TANK_HEIGHT  350.00   //in cm
  

volatile double duration, distance;
volatile double keroseneLevel, dieselLevel,  petrolLevel;
  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("The Level readings are below :");
  delay(1500);
  
  // Note: verify the following code 

  pinMode(DIESEL_TRIG, OUTPUT);    // Digital PIN 2
  pinMode(DIESEL_ECHO, INPUT);     // Digital PIN 3
  
  pinMode(PETRO_TRIG, OUTPUT);     // Digital PIN 4
  pinMode(PETRO_ECHO, INPUT);      // Digital PIN 5
  
  pinMode(KEROS_TRIG, OUTPUT);     // Digital PIN 6
  pinMode(KEROS_ECHO, INPUT);      // Digital PIN 7
  

  pinMode(CONNECT_WIFI_LED, OUTPUT);  // Digital PIN 12
  pinMode(CONNECT_HOST_LED, OUTPUT);  // Digital PIN 13
}

void loop() {

  /**
   * move to interrupt
   */
//  // if connected to wifi network 
//  blink_signal(CONNECT_WIFI_LED);
//
//  // if connected to host network
//  blink_signal(CONNECT_HOST_LED);
  
  
  // The following code is working to accept the level
  // of the Three materials
  dieselLevel = distance_detect(DIESEL_TRIG, DIESEL_ECHO, DIESEL_TANK_HEIGHT);
  keroseneLevel = distance_detect(KEROS_TRIG, KEROS_ECHO, KEROS_TANK_HEIGHT);
  petrolLevel = distance_detect(PETRO_TRIG, PETRO_ECHO, PETROL_TANK_HEIGHT);

  /**
   * Send data to web
   */


  // The below section is to print onto  the serial Monitor
  Serial.print("The Level of the Diesel tank is: ");
  print_level(dieselLevel);

  Serial.print("The Level of the Kerosene tank is: ");
  print_level(keroseneLevel);

  Serial.print("The Level of the Petrol tank is: ");
  print_level(petrolLevel);

}

double distance_detect(uint8_t trig_pin, uint8_t echo_pin, double tank_height)
{
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  duration = pulseIn(echo_pin, HIGH);
  distance = static_cast<double>(duration)/58.2;
  return (tank_height - distance);
}

void print_level(double level_value)
{
  Serial.println(level_value);
  _delay_ms(1000);
}

void blink_signal(uint8_t signal_pin )
{
  digitalWrite( signal_pin, HIGH);
  _delay_ms(1000);
  digitalWrite( signal_pin, LOW);
  _delay_ms(1000);
}


  

