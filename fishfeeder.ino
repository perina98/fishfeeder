#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include "Servo.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "user_interface.h"

const char * ssid = "*****";
const char * password = "*****";

const int SERVOGO = 40;
const int SERVOSTOP = 90;
const int SERVOBACK = 120;

int feedingTime = 15;

int lastFed = 0;

int FULLBOXTIME = 180;
int remainingTime = FULLBOXTIME;
int customfeeding = 0;

struct ActionTime {
  int hours;
  int minutes;
};

ActionTime actionTimeFirst = {6,0};
ActionTime actionTimeSecond = {18,0};

Servo myservo;  // create servo object to control a servo

// Setting up IP config (https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html)
IPAddress local_ip(192, 168, 1, 124);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

ESP8266WebServer server(80);

void setup() {
  
  Serial.begin(9600);
  myservo.attach(D1);
  
   // wifi_set_sleep_type(LIGHT_SLEEP_T);
  WiFi.config(local_ip, dns, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected");
  Serial.println("");

  
  server.on("/", connect);
  server.on("/feed", feed);
  server.on("/back", back);
  server.on("/first", setFirstTime);
  server.on("/second", setSecondTime);
  server.on("/duration", setDuration);
  server.on("/resetbox", resetBox);
  server.begin();

  // Start the Serial Monitor
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");
}

void connect() {
  if (server.args() > 0) {
    String s = server.arg(0);
    actionTimeFirst.hours = s.substring(0, 2).toInt();
    actionTimeFirst.minutes = s.substring(3, 5).toInt();
  }
  if(customfeeding > 0){
    remainingTime = remainingTime - ((millis() - customfeeding) / 1000);
    customfeeding = 0;
  }
  
  myservo.write(SERVOSTOP); 
  server.send(200, "text/html", html());
}

void resetBox(){
  remainingTime = FULLBOXTIME;
  server.send(200, "text/html", html());
}

void setFirstTime(){
  if (server.args() > 0) {
    String s = server.arg(0);
    actionTimeFirst.hours = s.substring(0, 2).toInt();
    actionTimeFirst.minutes = s.substring(3, 5).toInt();
  }
  myservo.write(SERVOSTOP); 
  server.send(200, "text/html", html());
}
void setSecondTime(){
  if (server.args() > 0) {
    String s = server.arg(0);
    actionTimeSecond.hours = s.substring(0, 2).toInt();
    actionTimeSecond.minutes = s.substring(3, 5).toInt();
  }
  myservo.write(SERVOSTOP); 
  server.send(200, "text/html", html());
}

void setDuration(){
  if (server.args() > 0) {
    String s = server.arg(0);
    feedingTime = s.toInt();
  }
  myservo.write(SERVOSTOP); 
  server.send(200, "text/html", html());
}


void feed() {
  myservo.write(SERVOBACK); 
  delay(1000);
  customfeeding = millis();
  myservo.write(SERVOGO); 
  server.send(200, "text/html", html());
}

void back() {
  myservo.write(SERVOBACK); 
  server.send(200, "text/html", html());
}

void feedTimed(){
  myservo.write(SERVOBACK); 
  delay(1000);
  myservo.write(SERVOGO);
  delay(feedingTime * 500);
  myservo.write(SERVOBACK); 
  delay(1000);
  myservo.write(SERVOGO);
  delay(feedingTime * 500);
  myservo.write(SERVOSTOP); 
}

void loop() {
  server.handleClient();
  time_t now = time(nullptr);
  tm * local = localtime( & now);

  if(((local->tm_hour == actionTimeFirst.hours) && (local->tm_min == actionTimeFirst.minutes) && local->tm_sec < 60) ||
      ((local->tm_hour == actionTimeSecond.hours) && (local->tm_min == actionTimeSecond.minutes) && local->tm_sec < 60)){
    if(millis() - lastFed > 90000){
      feedTimed();
      lastFed = millis();
      remainingTime = remainingTime - feedingTime;
    }
  }

  delay(1000);
}

String html() {
  time_t tnow = time(nullptr);
  String time = String(ctime( & tnow));

  int remainingFeedings = floor(remainingTime / feedingTime);
  int remainingDays = floor(remainingFeedings / 2);
  
  String ptr = "";
  ptr = "<!DOCTYPE html> <head> <title>FishFeeder</title> <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <style> * { margin: 0; padding: 0; border: 0; outline: 0; vertical-align: baseline; font-family: Arial, Helvetica, sans-serif; box-sizing: border-box; } body, html { padding: 0; margin: 0 } .content { max-width: 1000px; margin: 0 auto; margin-top: 3%; text-align: center; margin-bottom: 100px; } .button { background-color: dodgerblue; color: white; margin-top: 5px; height: 50px; text-align: center; text-decoration: none; font-size: 16px; cursor: pointer; width: 150px; display: inline-block; border: 1px solid white; border-radius: 15px; } .button:hover { background-color: cornflowerblue; } .title { text-align: center; margin-top: 3%; } .subtitle { text-align: left; margin-left: 10%; margin-top: 3%; } .time { text-align: right; margin-right: 5%; color: red; } .hrr { border-top: 2px solid dodgerblue; margin: 1% 0% 3% 0; } .actionbtns{ display: flex; justify-content:center; } .sensorData{ display: flex; justify-content: space-evenly; margin-top: 30px; } .sensorData div p{ color: red; } .watering{ margin-top: 30px; } .watering p span{color: red;} form{ margin-top: 20px; } form input, .setting{ padding: 10px; } @media only screen and (max-width: 600px) { .actionbtns{ flex-direction: column; align-items:center; } .button{ width: 200px; }  } #ftime {border: 1px solid gray;} </style> </head>";

  ptr += "<body>";
  ptr += "<h1 class=\"title\">FishFeeder</h1>";

  ptr += "<div class =\"content\"> <h3 class=\"subtitle\">Welcome</h3> <p class=\"time\">";
  ptr += time;
  ptr += "</p> <hr class=\"hrr\"> ";

  ptr += "<p> Zostáva " + String(remainingFeedings) + " krmení alebo " + String(remainingDays) + " dní</p>";

  ptr += "<div class=\"actionbtns\">";
  
  ptr += "<button class = \"button\" onclick = \"location.href='/'\">Aktualizovať</button>";
  ptr += "<button class = \"button\" onclick = \"location.href='/feed'\">Štart</button>";
  ptr += "<button class = \"button\" onclick = \"location.href='/'\">Stop</button>";
  ptr += "<button class = \"button\" onclick = \"location.href='/back'\">Back</button>";
  ptr += "<button class = \"button\" onclick = \"location.href='/resetbox'\">Resetovať box</button>";
  
  ptr += "</div>";

  ptr += "<form  action = \"/duration\"><h3>Dĺžka kŕmenia</h3> <input type=\"number\" id=\"ftime\" name=\"appt_dura\" required><hr><input type = \"submit\" class=\"button\"></form>";
  ptr += "<form  action = \"/first\"><h3>Prvý čas kŕmenia</h3> <input type=\"time\" id=\"apptf\" name=\"appt_one\" required><hr><input type = \"submit\" class=\"button\"></form>";
  ptr += "<form  action = \"/second\"><h3>Druhý čas kŕmenia</h3> <input type=\"time\" id=\"appt\" name=\"appt_two\" required><hr><input type = \"submit\" class=\"button\"></form>";
  ptr += "<div class=\"setting\">";
  ptr += " <p>Dĺžka: ";
  ptr += feedingTime;
  ptr += "s</p>";
  ptr += " <p>Prvý čas: ";
  ptr += actionTimeFirst.hours;
  ptr += ":";
  if (actionTimeFirst.minutes == 0) {
    ptr += "00";
  } else {
    ptr += actionTimeFirst.minutes;
  }
  ptr += ":00";
  ptr += "</p>";
  ptr += " <p>Druhý čas: ";
  ptr += actionTimeSecond.hours;
  ptr += ":";
  if (actionTimeSecond.minutes == 0) {
    ptr += "00";
  } else {
    ptr += actionTimeSecond.minutes;
  }
  ptr += ":00";
  ptr += "</p>";
  ptr += "</div></div>";
  ptr += "</body></html>";
  return ptr;

}
