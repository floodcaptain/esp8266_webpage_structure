#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h> 

#define _BUTTON_PIN 2

#define _AP_PASS "qwerty123"

String st_ssid = "";
String st_pass = "";
String deviceID    = "";

unsigned int cur_ms_count = 0;
unsigned int prev_ms_count = 0;

int ESP_AP_FLAG=0;
int AP_SERVER_CREATED=0;
ESP8266WebServer server(80);

void setup() {
  pinMode(_BUTTON_PIN ,INPUT); //enable button
  digitalWrite(_BUTTON_PIN , HIGH);  //enable pullup. give ground to this pin to activate .

  EEPROM.begin(100);  
  Serial.begin(9600);                 
  delay(2000);

  /*** code block to obtain deviceID from MAC***/
  deviceID=WiFi.macAddress();
  deviceID.remove(11,16);deviceID.remove(2,1);deviceID.remove(4,1);deviceID.remove(6,1);
  Serial.print("MAC: ");Serial.print(deviceID);
  /********************************************/

  //read_wifi_sta_credentials();

  st_wifi_connect(100);

  delay(2000);
}

void loop() {
  if(!ESP_AP_FLAG){    //when esp is in st mode

    /**** code block to check wifi available and connect while in AP mode ***/
    cur_ms_count = millis();
    if((unsigned long)(cur_ms_count - prev_ms_count)>10000){
      prev_ms_count = cur_ms_count;
      if(st_wifi_connect(100)){
        WiFi.mode(WIFI_STA);
        ESP_AP_FLAG=0;
        AP_SERVER_CREATED=1;
      }
    } 
    /************************************************************************/

  if(ESP_AP_FLAG){    //when esp is in ap mode
    if(st_wifi_connect(100)){
      WiFi.mode(WIFI_STA);
      ESP_AP_FLAG=0;
      AP_SERVER_CREATED=1;
    delay(1000);
    }
    if(!AP_SERVER_CREATED){
    /* this code fragment creates AP SERVER */
      WiFi.disconnect();
      start_server(deviceID,_AP_PASS);
      AP_SERVER_CREATED=1;
      ESP_AP_FLAG=1;
      Serial.println("hotspot created");
    }
    server.handleClient();
  }
}
}

/*  
  @function: clears eeprom till data stored .
  @param: api_path: path or url of API eg. api.abc.com:8081/path
          post_message: message content . 
  @retval : 0 if wifi not connected.
            2xx/3xx if data sent
            4xx if data sending failed
*/
int send_api_data(String api_path,String post_message)
{
  if(WiFi.status()!= WL_CONNECTED)
    {return 0;}

  HTTPClient http; 

  http.begin(api_path); 
  http.addHeader("Content-Type", "text/plain");

   int httpCode = http.POST(post_message);   //Send the request
   delay(1);
   String payload = http.getString();                  //Get the response payload

   http.end();  //Close connection
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload

 }

/*  
  @function: connect esp8266 to saved wifi .
  @param: try_count: number of maximum tries to attempt to connect to wifi .
  @retval : 1 if successfully created and configured.
            0 if connection failed .
  @NOTE: module is restarted if configuration is failed .
*/
volatile bool st_wifi_connect(volatile int try_count)
{
  volatile int st_connect_counter=0;
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(st_ssid.c_str(),st_pass.c_str());
  Serial.println("attempting to connect to wifi...");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(100);
    if(++st_connect_counter>try_count)
      return 0;
  }
  
  if (!MDNS.begin(deviceID.c_str())) {             // Start the mDNS responder for /deviceID/.local
    Serial.println("Error setting up MDNS responder!");
    return 0;
  }
  Serial.println("wifi connected");
  return 1;
}

/*  
  @function: starts esp8266 in AP/hotspot mode .
  @param: _SSID: _SSID for esp8266 AP .
          _PASS: password for esp8266 AP .
          int: channel for AP.
  @retval : 1 if seuccessfully created and configured.
  @NOTE: module is restarted if configuration is failed .
*/
 void start_server(String ssid, String pass)
 {

  ESP_AP_FLAG = 1;
  Serial.println("setting up hotspot");
  IPAddress local_IP(192,168,4,2);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);

  WiFi.mode(WIFI_AP_STA);
  if(!WiFi.softAPConfig(local_IP, gateway, subnet))
  {
    reset_module("failed to configure AP");
  }

  if(!WiFi.softAP(ssid.c_str() , pass.c_str()))
  {
    reset_module("failed to setup AP");
  }
  else{Serial.println(ssid);Serial.println(pass);}
/**************************SERVER ROUTES*****************/
  server.on("/",homepage);
  server.on("/change_wifi_cred",change_wifi_cred);
/********************************************************/
  server.begin();
  Serial.println("   ");
  Serial.println("   ");
  Serial.println("SERVER CREATED. IP: ");
  Serial.println("   ");
  Serial.println(WiFi.softAPIP());
}

/*  
  @function: creates hotspot and call related functions if button press detected .
  @param: none
  @retval : none
*/
void check_button_press()
{
  if(digitalRead(!_BUTTON_PIN))
  {
    delay(10);
    if(digitalRead(!_BUTTON_PIN)){
      Serial.println("button pressed");
      WiFi.disconnect();
      ESP_AP_FLAG =1;
      start_server(deviceID,_AP_PASS);
      delay(100);
    }
  }  
}

/*  
  @function: resets esp8266 .
  @param: error_message : error message to log before restarting
  @retval : none
*/
void reset_module(String error_message)
{ 
  Serial.println("  ");
  Serial.println(error_message);
  Serial.println("  ");
  Serial.println("  ");
  Serial.println("restarting device in: ");
  Serial.println("1");
  Serial.println("  ");
  Serial.println("2");
  Serial.println("  ");
  Serial.println("3");
  ESP.restart();
  //ESP.reset();
  Serial.println("please manually reset the device");
  while(1);
}

/*  
  @function: route for homepage .
  @param: none
  @retval : none
*/
void homepage()
{
  String index_html="";
  WiFi.scanDelete();
  int Tnetwork = WiFi.scanNetworks();//Scan for total networks available
  String ssid_entry = "<ul>";
  for (int i = 0; i < Tnetwork; ++i)
    {
      // Print SSID 
      ssid_entry += "<li><form action='/change_wifi_cred' method='post'><label>SSID: </label><input name='ssid' length=32 value=\"";
      ssid_entry += WiFi.SSID(i);
      ssid_entry +="\"/>";
      ssid_entry += "<label>Pasword: </label><input name='pass' length=64><input type='submit'></li></form><br>";
    }
      ssid_entry += "</ul>";
      index_html= "\n\r\n<!DOCTYPE HTML>\r\n<html><h1> AVAILABLE ACCESS POINTS:</h1> ";
      index_html += "<p>";
      index_html += ssid_entry;
     // index_html += "<form method='get' action='a'><label>SSID: </label><input name='ssid' length=32><label>Pasword: </label><input name='pass' length=64><input type='submit'></form>";
      index_html += "</p>";
      index_html += "</html>";
      
    server.send( 200 , "text/html", index_html);
}

/*  
  @function: route for when user key in password and redirects
             to final page upon success .
  @param: none
  @retval : none
*/

void change_wifi_cred(){ 
  int i=0;
  String pass_received ="",ssid_received="",finalstring="";
  if(server.args()==2){
    ClearEeprom();
    ssid_received = server.arg(0);
    pass_received = server.arg(1);
    char buffer[100];
    finalstring+=ssid_received;
    finalstring+='\0';
    finalstring+=pass_received;
    finalstring+='\0';

    Serial.println("     ");
    Serial.println("FINAL STRING: ");
    Serial.println(finalstring);
    Serial.println("        ");

    while(i<finalstring.length())
    {
      EEPROM.write(i,finalstring[i]);
      ++i;
    }
    EEPROM.commit();
    EEPROM.end();

    delay(100);
    Serial.println("    ");
    read_wifi_sta_credentials();

    String temp = "\
    <html>\
    <head>\
    <title>success</title>\
    <body>\
    <h1>credentials saved successfully</h1>\
    <p>your module is being restarted</p>\
    <br><p>Your entered credentials are</p>\
    <p>ACCESS POINT: <strong>";
    temp+=ssid_received;
    temp+="</strong></p><p>PASSWORD: <strong>";
    temp+=pass_received;
    temp+="</strong></p>\
    </body>\
    </head>\
    </html>";
    server.send(200,"text/html",temp);

    WiFi.mode(WIFI_STA);
    ESP_AP_FLAG =0;AP_SERVER_CREATED=1;
    reset_module("restarting: new wifi configured");
  }
 reset_module("received more than 2 post parameters");
}

/*****************EEPROM ROUTINES*******************/
void read_wifi_sta_credentials()
{
  int i=0;
  while(char(EEPROM.read(i)))
  {
    st_ssid += char(EEPROM.read(i));
    ++i;
  }

  ++i;
  while(char(EEPROM.read(i)))
  {
    st_pass += char(EEPROM.read(i));
    ++i;
  }

  Serial.println("      ");
  Serial.println("SSID: ");
  Serial.println(st_ssid); 
  Serial.println("      ");
  Serial.println("PASS: ");
  Serial.println(st_pass);
}

void ClearEeprom(){
  Serial.println("Clearing Eeprom");
  for (int i = 0; i < 96; i++) { EEPROM.write(i,'\0'); }
  EEPROM.commit();
  
  Serial.println("       ");
  Serial.println("reading Eeprom: ");
  for (int i = 0; i < 96; i++) { Serial.print(EEPROM.read(i)); }
}

/****************************************************/

