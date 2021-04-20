/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com  
*********/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include "Servo.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
const char *ssid = "Home_2G";
const char *password = "M7ze!2XyXg";
int servo_pin = D7;
Servo myservo;
int angle = 0;
int past_angle = 0;
String last = "Nie";
String last_t = "Nie";
int hours = 20;
int minutes = 0;
bool fed = false;
float temperatureC_s1 = 0;
float temperatureC_s2 = 0;


// Setting up IP config (https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html)
IPAddress local_ip(192, 168, 1, 222);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

ESP8266WebServer server(80);

// GPIO where the DS18B20 is connected to
const int oneWireBus = D3;     
const int secondWireBus = D6;    

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
OneWire secondWire(secondWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensor1(&oneWire);
DallasTemperature sensor2(&secondWire);

void setup() {
      WiFi.config(local_ip,dns, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); 
server.on("/", connect);
    server.on("/generate", generate);
    server.begin();

    
  // Start the Serial Monitor
  Serial.begin(115200);
  // Start the DS18B20 sensor
  sensor1.begin();
  sensor2.begin();
  // set up the LCD's number of columns and rows:
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    lcd.begin();
lcd.noBacklight();
  lcd.clear();
  
    myservo.attach(servo_pin);
    myservo.write(0);
    myservo.detach();
}

void connect()
{
  if (server.args() > 0)
    {
        String s = server.arg(0);
        hours = s.substring(0,2).toInt();
        minutes = s.substring(3,5).toInt();
    }
  time_t tnow = time(nullptr);
    server.send(200, "text/html", html(0,String(ctime(&tnow))));
}


void generate()
{
  time_t tnow = time(nullptr);
    angle = server.arg(0).toInt();
    if (angle == 360)
    {
        myservo.attach(servo_pin);
       for (int i = 0; i <= 90; i++)
        {
            myservo.write(i);
            delay(1);
        }
        delay(400);
        for (int i = 90; i >= 0; i--)
        {
            myservo.write(i);
            delay(1);
        }
         last_t = String(ctime(&tnow));
        tm* local = localtime(&tnow);
        last = String(local->tm_hour);
        last += ":";
        last += String(local->tm_min);
        myservo.detach();
    }
    else
    {
      myservo.attach(servo_pin);
        for (int i = 0; i <= angle; i++)
        {
            myservo.write(i);
            delay(10);
        }
        myservo.detach();
    }

    server.send(200, "text/html", html(0,String(ctime(&tnow))));
}


void loop() {
  server.handleClient();
  time_t now = time(nullptr);
  sensor1.requestTemperatures(); 
  sensor2.requestTemperatures();
  temperatureC_s1 = sensor1.getTempCByIndex(0);
  temperatureC_s2 = sensor2.getTempCByIndex(0);
  float temperatureF = sensor1.getTempFByIndex(0);
  
  tm* local = localtime(&now);

  if(local->tm_sec < 2){
    lcd.clear();
    lcd.print("Aktualny Cas: ");
    lcd.print(local->tm_hour);
    lcd.print(":");
    if(local->tm_min == 0){
      lcd.print("00");
      }else{
        if(local->tm_min < 10){
          lcd.print("0");
          }
    lcd.print(local->tm_min);}
   lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temperatureC_s1);
    lcd.print("/");
    lcd.print(temperatureC_s2);
    lcd.print(" C");
     lcd.setCursor(0, 2);
     lcd.print("Posledne: ");
     lcd.print(last);
     lcd.setCursor(0, 3);
     lcd.print("Planovane: ");
     lcd.print(hours);
      lcd.print(":");
      if(minutes == 0){
        lcd.print("00");
        } else{
    lcd.print(minutes);
        }
  }

  
 if(local->tm_hour == hours && local->tm_min == minutes+1){
  fed = false;
  }

  if(local->tm_hour == hours && local->tm_min == minutes &&  local->tm_sec < 2){
    fed = true;
    myservo.attach(servo_pin);
       for (int i = 0; i <= 90; i++)
        {
            myservo.write(i);
            delay(1);
        }
        delay(400);
        for (int i = 90; i >= 0; i--)
        {
            myservo.write(i);
            delay(1);
        }
        last_t = String(ctime(&now));
        tm* local = localtime(&now);
        last = String(local->tm_hour);
        last += ":";
        last += String(local->tm_min);
        myservo.detach();
      }
}

String html(int status, String time)
{
  
    String ptr = "";
    ptr = "<html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"><style>*{margin: 0; padding: 0; border: 0;outline: 0; vertical-align: baseline; font-family: Arial, Helvetica, sans-serif; box-sizing: border-box;}body,html{padding: 0; margin: 0}.content{max-width: 1000px; margin: 0 auto; margin-top: 3%;text-align:center;margin-bottom:100px;}.leds{text-align:center;display:inline-grid;}.button{background-color: dodgerblue; color: white; margin-top: 5px; height:50px; text-align: center; text-decoration: none; font-size: 16px; cursor: pointer; width: 150px; display: inline-block;border: 1px solid white;border-radius:15px;}.button:hover{background-color: cornflowerblue;}.title{text-align: center; margin-top: 3%;}.hrr{border-top: 2px solid dodgerblue; margin: 1% 0% 3% 0;}.loader{border: 16px solid white; border-radius: 50%; border-top: 16px solid dodgerblue; border-bottom: 16px solid dodgerblue; width: 120px; height: 120px; -webkit-animation: spin 2s linear infinite; animation: spin 2s linear infinite; position: fixed; top: 30%; left: 35%;}@-webkit-keyframes spin{0%{-webkit-transform: rotate(0deg);}100%{-webkit-transform: rotate(360deg);}}@keyframes spin{0%{transform: rotate(0deg);}100%{transform: rotate(360deg);}}.sels{ background: lightgray; border-radius: 5px; width: 100%;margin: 5px;display:block;height:40px;}@media only screen and (min-width:900px){.leds{display:inline-flex;}.loader{left: 47%;}}.subtitle{text-align:left;margin-left:10%;margin-top:3%;}.setup{display:inline-block;text-align:center;}</style> </head>";

    ptr += " <body>";
    ptr += "<h1 class=\"title\">FishFeeder Controll App</h1>";

    ptr += "<div class =\"content\"> <h3 class=\"subtitle\">Welcome</h3> <hr class=\"hrr\"> <div class=\"setup\">";
ptr += "<button class = \"button\" onclick = \"location.href='/'\"> Reload page</button>";
    ptr += "<div class=\"\">";
    ptr += "<button class = \"button\" onclick = \"location.href='/generate?command=360'\"> Feed the fish</button>";
    ptr += "<button class = \"button\" onclick = \"location.href='/generate?command=0'\"> Close</button>";
    ptr += "<button class = \"button\" onclick = \"location.href='/generate?command=180'\"> Open</button>";
    ptr += "<div class=\"\" style=\"margin-top: 30px;\">";
  ptr += "<h2> Time now is</h2> <p style=\"color:red\">";
  ptr += time;
  ptr += "</p>";
  ptr += "</div>";
  ptr += "<div class=\"\" style=\"margin-top: 30px;\">";
  ptr += "<h2> Current temperature</h2> <p style=\"color:red\">";
  ptr += temperatureC_s1;
  ptr += " C / ";
  ptr += temperatureC_s2;
  ptr += " C</p>";
  ptr += "</div>";
  ptr += "<div class=\"\"  style=\"margin-top: 30px;\">";
  ptr += "<h2> Last feeding was at </h2> <p style=\"color:red\"> ";
  ptr += last_t;
  ptr += "</p>";
  ptr += "</div>";
  ptr += "</div>";
  ptr += "<form  style=\"margin-top: 20px;\" action = \"/\"><h3>Feed every day at</h3> <input type=\"time\"  style=\"padding: 10px;\" id=\"appt\" name=\"appt\"";
  ptr += " required><hr><input type = \"submit\" class=\"button\"></form>";
  ptr += "<div class=\"\"  style=\"margin-top: 10px;\">";
  ptr += " <p> Current setting : ";
  ptr += hours;
  ptr += ":";
  if(minutes == 0){
    ptr += "00";
    } else {
      ptr += minutes;
      }
  
  ptr += ":00";
  ptr += "</p>";
    ptr += "</div>";
    ptr += "</body></html>";
    return ptr;
}
