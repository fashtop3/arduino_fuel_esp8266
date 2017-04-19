#include <SoftwareSerial.h>
#include <util/delay.h>

#define SSID        "ADYbWU";//"SUCCESS"                                        // Wifi SSID
#define PASS        "nokia66542";//"olatundeayo44"                                    // WiFi Password
#define DEST_HOST   "api.wunderground"                            // API site host
#define DEST_IP     "192.254.235.21"                              // API site IP address
#define TIMEOUT     5000                                          // mS
#define CONTINUE    false                                         // Define for readability
#define HALT        true                                          // Define for readibility


boolean echoCommand(String, const char *, boolean, int);
void echoSkip();
boolean expect(const char *, uint16_t);
boolean connectWiFi();
void errorHalt();

int buf_size = 64;
volatile char serialbuffer[1000];//serial buffer for request url
SoftwareSerial ESP8266(10,11); // RX (goes to TX on ESP), TX (goes to RX on ESP)


void setup()
{
  ESP8266.begin(9600); //connection to ESP8266
  Serial.begin(9600); //serial debug

//  ESP8266.setTimeout(TIMEOUT);              // Set the timeout value for the ESP8266
  
  //set mode needed for new boards

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

//    String cmd = "+CIPSTART=\"TCP\",\""; cmd += "erp.rockcityfmradio.com"; cmd += "\",80"; // Establish TCP connection
    String cmd = "+CIPSTART=\"TCP\",\""; cmd += "192.168.43.103"; cmd += "\",80"; // Establish TCP connection
    char host_buf[100]; 
    cmd.toCharArray(host_buf, 100);
    _delay_ms(2000);
    Serial.println("Connecting to Host...!!!");
    Serial.println(host_buf);

    while(!expect_AT_OK(host_buf/*"+CIPSTART=\"TCP\",\"192.168.43.94\",80"*/, 6000)) {
//      expect_scan(F("+SAPBR: 1,%hu,\"%*hu.%*hu.%*hu.%*hu\""), &status, 3000); //+SAPBR: 1,1,"10.96.42.184"
      //Todo: limit loop
      _delay_ms(7000);
      Serial.println("Connecting to Host...!!!");
      expect_AT_OK("+CIPCLOSE", 3000);
      eat_echo();
    }

    Serial.println("Connected to Host...!!!");
    delay(2000);                            // Indicate there was an error
  }

}

void loop()
{
  //output everything from ESP8266 to the Arduino Micro Serial output
//  while (ESP8266.available() > 0) {
//    Serial.write(ESP8266.read());
//  }
//  
//  if (Serial.available() > 0) {
//     //read from serial until terminating character
//     int len = Serial.readBytesUntil('\n', serialbuffer, sizeof(serialbuffer));
//  
//     //trim buffer to length of the actual message
//     String message = String(serialbuffer).substring(0,len-1);
//     Serial.println("message: " + message);
// 
//     //check to see if the incoming serial message is a url or an AT command
//     if(message.substring(0,2)=="AT"){
//       //make command request
//       Serial.println("COMMAND REQUEST");
//       ESP8266.println(message); 
//     }else{
//      //make webrequest
//       Serial.println("WEB REQUEST");
//       WebRequest(message);
//     }
//  }
}

////web request needs to be sent without the http for now, https still needs some working
//void WebRequest(String request){
// //find the dividing marker between domain and path
//     int slash = request.indexOf('/');
//     
//     //grab the domain
//     String domain;
//     if(slash>0){  
//       domain = request.substring(0,slash);
//     }else{
//       domain = request;
//     }
//
//     //get the path
//     String path;
//     if(slash>0){  
//       path = request.substring(slash);   
//     }else{
//       path = "/";          
//     }
//     
//     //output domain and path to verify
//     Serial.println("domain: |" + domain + "|");
//     Serial.println("path: |" + path + "|");     
//     
//     //create start command
//     String startcommand = "+CIPSTART=\"TCP\",\"" + domain + "\", 80"; //443 is HTTPS, still to do
//     
//     ESP8266.println(startcommand);
//     Serial.println(startcommand);
//     
//     
//     //test for a start error
//     if(ESP8266.find("Error")){
//       Serial.println("error on start");
//       return;
//     }
//     
//     //create the request command
//     String sendcommand = "GET http://"+ domain + path + " HTTP/1.0\r\n\r\n\r\n";//works for most cases
//     
//     Serial.print(sendcommand);
//     
//     //send 
//     ESP8266.print("+CIPSEND=");
//     ESP8266.println(sendcommand.length());
//     
//     //debug the command
//     Serial.print("+CIPSEND=");
//     Serial.println(sendcommand.length());
//     
//     //delay(5000);
//     if(ESP8266.find(">"))
//     {
//       Serial.println(">");
//     }else
//     {
//       ESP8266.println("+CIPCLOSE");
//       Serial.println("connect timeout");
//       delay(1000);
//       return;
//     }
//     
//     //Serial.print(getcommand);
//     ESP8266.print(sendcommand); 
//}


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
  
  if (expect_AT_OK(cmd_buf, 8000))                                               // Join Access Point
    return true;
  else
    return false;
}

void errorHalt()
{
  while(true){};
}

