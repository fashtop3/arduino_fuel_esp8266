#include <SoftwareSerial.h>
#include <util/delay.h>

#define SSID        "ADYbWU";//"SUCCESS"                                        // Wifi SSID
#define PASS        "nokia66542";//"olatundeayo44"                                    // WiFi Password
                         // mS
#define STATION_ID  1 // station ID

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
  

/**
 * Global variable declearation
 */
volatile double duration, distance;
volatile double keroseneLevel, dieselLevel,  petrolLevel;

int buf_size = 64;
const String HOST = "192.168.43.103";
const int PORT = 8000;


/**
 * function protypes
 */
boolean echoCommand(String, const char *, boolean, int);
void echoSkip();
boolean expect(const char *, uint16_t);
boolean connectWiFi();
void errorHalt();
void connectToHost(String, int);

SoftwareSerial ESP8266(10,11); // RX (goes to TX on ESP), TX (goes to RX on ESP)


void setup()
{

  ultra_module_config();
  wifi_setup();
}

void loop()
{

//  while (ESP8266.available() > 0) {
//      Serial.write(ESP8266.read());
//    }

    eat_echo();

    // The following code is working to accept the level
    // of the Three materials
    dieselLevel = distance_detect(DIESEL_TRIG, DIESEL_ECHO, DIESEL_TANK_HEIGHT);
    keroseneLevel = distance_detect(KEROS_TRIG, KEROS_ECHO, KEROS_TANK_HEIGHT);
    petrolLevel = distance_detect(PETRO_TRIG, PETRO_ECHO, PETROL_TANK_HEIGHT);

    // The below section is to print onto  the serial Monitor
    Serial.print("The Level of the Diesel tank is: ");
    print_level(dieselLevel);
  
    Serial.print("The Level of the Kerosene tank is: ");
    print_level(keroseneLevel);
  
    Serial.print("The Level of the Petrol tank is: ");
    print_level(petrolLevel);
    
    WebRequest(petrolLevel, keroseneLevel, dieselLevel);
    delay(3000);

    reConnectToHost(HOST, PORT);
}

/**
 * setting up wifi module connections
 */
void wifi_setup()
{
  ESP8266.begin(9600); //connection to ESP8266
  Serial.begin(9600); //serial debug
  
  _delay_ms(1000);
  Serial.println("Restarting...");
  if(expect_AT("+RST", "OK", 2000)) {
     Serial.println("Restarted...");     
  }

  _delay_ms(2000);
  
  eat_echo();
  
  expect_AT_OK("E0", 2000);
  eat_echo();
    
//  echoCommand("+RST", "ready", CONTINUE);
  expect_AT_OK("+CWMODE=1", 6000);    // Station mode

  expect_AT_OK("+CIPMUX=0", 6000);    // Allow multiple connections (we'll only use the first).

  boolean connection_established = false;  // Initialize connection state


  expect_AT_OK("+CWQAP", 3000);
  _delay_ms(500);
  
  if (!expect_AT_OK("+CIPSTATUS", 6000)){
      Serial.println("Not connected...!!!");
      //connect to wifi network
      for(int i=0;i<5;i++)                     // For 5 attempts
      {
        Serial.println("Connecting...");
        connectWiFi();
        _delay_ms(4000);
        if (expect_AT_OK("+CIPSTATUS", 6000)){
            Serial.println("Connected!!!");
            connection_established = true;       // Set connection state
            break;                               // Break connection loop
        }  
      }
    }

    expect_AT_OK("+CIPCLOSE", 3000);
    _delay_ms(3000);
    
  if (connection_established)   {           // If connection attempts failed

      reConnectToHost(HOST, PORT);
      delay(2000);                            // Indicate there was an error
  }
}

void print_level(double level_value)
{
  Serial.println(level_value);
  _delay_ms(1000);
}

void ultra_module_config()
{
  pinMode(DIESEL_TRIG, OUTPUT);    // Digital PIN 2
  pinMode(DIESEL_ECHO, INPUT);     // Digital PIN 3
  
  pinMode(PETRO_TRIG, OUTPUT);     // Digital PIN 4
  pinMode(PETRO_ECHO, INPUT);      // Digital PIN 5
  
  pinMode(KEROS_TRIG, OUTPUT);     // Digital PIN 6
  pinMode(KEROS_ECHO, INPUT);      // Digital PIN 7
  

  pinMode(CONNECT_WIFI_LED, OUTPUT);  // Digital PIN 12
  pinMode(CONNECT_HOST_LED, OUTPUT);  // Digital PIN 13
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

//web request needs to be sent without the http for now, https still needs some working
void WebRequest(double petro, double kero, double diesel){

     //create the request command
     String sendcommand = "GET /api/station/"; sendcommand += STATION_ID;   
     sendcommand += "/data/"; 
     sendcommand += petro; 
     sendcommand += "/";
     sendcommand += kero; 
     sendcommand +="/"; 
     sendcommand += diesel; 
     sendcommand += " HTTP/1.0\r\n\r\n";
     
     //send 
     ESP8266.print("AT+CIPSEND=");
     ESP8266.println(sendcommand.length());
     
     //debug the command
     Serial.print("AT+CIPSEND=");
     Serial.println(sendcommand.length());
        
     if(ESP8266.find(">"))
     {
       Serial.print("> ");
       ESP8266.print(sendcommand);
       Serial.print(sendcommand);
       _delay_ms(3000);
        eat_echo();
     }
}


void connectToHost(String host, int port)
{

      expect_AT_OK("+CIPCLOSE", 3000);
      _delay_ms(3000);
      String cmd = "+CIPSTART=\"TCP\",\""; cmd += host; cmd += "\","; cmd += port; // Establish TCP connection
      char host_buf[100]; 
      cmd.toCharArray(host_buf, 100);
      _delay_ms(2000);
      Serial.println("Connecting to Host...!!!");
      Serial.println(host_buf);
  
      while(!expect_AT_OK(host_buf/*"+CIPSTART=\"TCP\",\"192.168.43.94\",80"*/, 6000)) {
        _delay_ms(5000);
        Serial.println("Connecting to Host...!!!");
        expect_AT_OK("+CIPCLOSE", 3000);
        eat_echo();
    }

    Serial.println("Connected to Host...!!!");
}

void reConnectToHost(const String host, const int port)
{
      expect_AT_OK("+CIPCLOSE", 3000);
      _delay_ms(1000);
      String cmd = "+CIPSTART=\"TCP\",\""; cmd += host; cmd += "\","; cmd += port; // Establish TCP connection
      char host_buf[100]; 
      cmd.toCharArray(host_buf, 100);
      Serial.println("Connecting to Host...!!!");
      Serial.println(host_buf);
  
      while(!expect_AT_OK(host_buf/*"+CIPSTART=\"TCP\",\"192.168.43.94\",80"*/, 6000)) {
        _delay_ms(5000);
        Serial.println("Connecting to Host...!!!");
        expect_AT_OK("+CIPCLOSE", 3000);
        eat_echo();
    }

    Serial.println("Connected to Host...!!!");
}

bool expect_AT(const char *cmd, const char *expected, uint16_t timeout)
{
  ESP8266.print("AT");
  ESP8266.println(cmd);
  if (expected == "")  {               // If no ack response specified, skip all available module output.
    eat_echo();    
  } 
  return expect(expected, timeout);
}

bool expect_AT_OK(const char *cmd, uint16_t timeout)
{
  return expect_AT(cmd, "OK", timeout);
}

bool expect_scan(const char *pattern, void *ref, uint16_t timeout)
{
  char buf[buf_size];
  size_t len;
  readline(buf, buf_size, timeout);
  return sscanf(buf, (const char *) pattern, ref) == 1;
}


boolean expect(const char *expected, uint16_t timeout)
{
  uint16_t idx = 0;
  char buf[buf_size];
  size_t len;

//  eat_echo();
  len = readline(buf, buf_size, timeout);
   
  return strcmp(buf, (const char *) expected) == 0;
}

int readline(char *buf, size_t buf_size, uint16_t timeout)
{
  uint16_t idx = 0;
  while(--timeout)                       // Try until deadline
  {
    while (ESP8266.available())                        // If characters are available
    {
      char ch = ESP8266.read();
      Serial.write(ch);
      if (ch == '\r') continue;
      if (ch == '\n') {
        if (!idx) continue;
            timeout = 0;
            break;
        }
        if (buf_size - idx) buf[idx++] = ch;
    }
    
    if (timeout == 0) break;
     _delay_ms(1);
  }

  buf[idx] = 0;
  return idx;
}


void eat_echo()
{
//  _delay_ms(200);
   while (ESP8266.available())
   {
    ESP8266.read();
    // don't be too quick or we might not have anything available
    // when there actually is...
    _delay_ms(1);
   }
}


boolean connectWiFi()
{
  String cmd = "+CWJAP=\""; cmd += SSID; cmd += "\",\""; cmd += PASS; cmd += "\"";  // Connection string to connect to wifi
  char cmd_buf[100]; 
  cmd.toCharArray(cmd_buf, 100);
  
//  Serial.println(cmd_buf);
  
  if (expect_AT_OK(cmd_buf, 8000))  {
    // Join Access Point
    eat_echo();
    return true;
  }
  else
    return false;
}

void errorHalt()
{
  while(true){};
}

