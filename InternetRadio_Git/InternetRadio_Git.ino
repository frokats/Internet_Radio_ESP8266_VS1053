
//Include Section
#include "spiffs/spiffs.h"
#include <FS.h>
#include <ESP8266mDNS.h>
#include <VS1053.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>


//Variables Section
const char* APssid     = "InternetRadio";
const char* APpassword = "123445767890";
const char* ssid     = "Your AP ssid";       //Set your AP ssid
const char* password = "Your AP Password";   //Set your AP password
int BassBoost=0;
int BassFreq=20;
int TrebleBoost=0;
int TrebleFreq=1;
int SysInt=0;
int RPort=80;
int NextRadioNum=0;
int AllRadioNum=0;
int Hour=0;
int Min=0;
int Sec=0;
int CHour=0;
int CMin=0;
int CSec=0;
int AlarmHour=0;
int AlarmMin=0;
String VOLUME="80";
String TONE="0";
String AllGenre;
String AllCountry;
String AllCity;
String HttpString;
String TempString;
String TempString2;
String TempString3;
String TempString4;
String TimeServer="google.com";
String Mp3Song;
String CRadioNum;
String RadioNum;
String RadioName;
String RadioUrl; 
String RadioPort;
String RadioCountry;
String RadioCity;
String RadioFile;
String RadioGenre; 
String PreviusRadioPort;
String PreviusRadioUrl;
String PreviusRadioFile;
String PreviusRadioCity;
String PreviusRadioName;
String PreviusRadioCountry;
String ssid_temp="InternetRadio";
String password_temp="123445767890";
String StartStop = "Stop";
String DateString;
String TimeString;
String AlarmTime;
String AlarmSong;
String AlarmAfter;
String Version="Internet Radio V1.0";
String Bottom="May the force be with you";
byte Tone[4];
byte Screen=1;
byte WifiConnectAttempts=0;
byte MaxAttempts=20;
byte MaxTrys=4;
byte clientSS = 1;
byte Alarm=0;
unsigned char Data[32];
unsigned long TimeNow;
unsigned long TimeLast;
unsigned long TimePlay;

IPAddress StaticIp(192, 168, 1, 249);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

/*Define Section
//Wiring of VS1053 board (SPI connected in a standard way) to Wemos D1 Mini
//VS1053_CS   D8  (15)
//VS1053_DCS  D0  (16)
//VS1053_DREQ D3  (0)
//MOSI        D7  (13)
//MISO        D6  (12)
//SCK         D5  (14)
//XRST        RST
*/
#define VS1053_CS     15
#define VS1053_DCS    16
#define VS1053_DREQ   0

//Constructors  Section
VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
WiFiClient client;
ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(0x3f,20,4);  
File fsUploadFile;

void setup() 
{
  Serial.begin(9600);
  delay(1);
  lcd.init();
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(Version);
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print("Initialization...");
  SPIFFS.begin();
  AllGenre=OptionAdd("genre");
  AllCountry=OptionAdd("countries");
  AllCity=OptionAdd("cities");
  SPI.begin();
  player.begin();
  player.switchToMp3Mode(); // optional, some boards require this
  VOLUME = LoadDataFromFile("VOL");
  if (VOLUME != "-1")
  {
    player.setVolume(VOLUME.toInt());
  }
  else
  {
    player.setVolume(80);
    VOLUME = "80";
  }
  if (SPIFFS.exists("/TrebleBoost.dat"))
  {
    TrebleBoost = LoadDataFromFile("TrebleBoost").toInt();
    TrebleFreq = LoadDataFromFile("TrebleFreq").toInt();
    BassBoost = LoadDataFromFile("BassBoost").toInt();
    BassFreq = LoadDataFromFile("BassFreq").toInt();
  }  
  Tone[0]=TrebleBoost;
  Tone[1]=TrebleFreq;
  Tone[2]=BassBoost;
  Tone[3]=BassFreq/10;
  player.setTone(Tone);
  Serial.println("Start...");
  WiFi.mode(WIFI_AP_STA);
  ssid_temp=LoadDataFromFile("StSSID");
  password_temp=LoadDataFromFile("StPass");
  Serial.println(ssid_temp + password_temp);
  if (ssid_temp == "-1")
  {
    WiFi.softAP(APssid);//, APpassword); whithout password
  }
  else
  {
    WiFi.softAP(ssid_temp.c_str());//, password_temp.c_str()); whithout password 
  }
  
  server.on("/", handleRoot);
  server.on("/Stations", handleStations);
  server.on("/PeekStation", handlePeekStation);
  server.on("/ViewStations", handleViewStation);
  server.on("/Wifi", handleWifi);
  server.on("/APSettings", handleAPSettings);
  server.on("/StSettings", handleStSettings);
  server.on("/Sound", handleSound);
  server.on("/Volume", handleVolume);
  server.on("/Bass", handleBass);
  server.on("/Treble", handleTreble);
  server.on("/StationChoice", handleStationChoise);
  server.on("/StartStop", handleStartStop);
  server.on("/NewStation", handleNewStation);
  server.on("/TestStation", handleTestStation);
  server.on("/SaveStation", handleAlarm);
  server.on("/Settings", handleSettings);
  server.on("/Alarm", handleAlarm);
  server.on("/AlarmSet", handleAlarmSet);
  server.on("/AlarmSettings", handleAlarmSettings);
  server.on("/AlarmSettings2", handleAlarmSettings2);
  server.on("/MonitorStation", handleMonitorStation);
  server.on("/FileUpload", HTTP_POST,[](){ server.send(200); }, handleFileUpload);
  server.begin();
  ssid_temp=LoadDataFromFile("APSSID");
  password_temp=LoadDataFromFile("APPass");
  Serial.println(ssid_temp + password_temp);
  if (ssid_temp == "-1")
  {
    WiFi.begin(ssid,password);
  }
  else
  {
    WiFi.begin(ssid_temp.c_str(), password_temp.c_str());   
  }
  WiFi.config(StaticIp, gateway, subnet);
  WifiConnectAttempts=0;
  lcd.setCursor(0,2);
  lcd.print("Connecting to AP...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    WifiConnectAttempts +=1;
    if(WifiConnectAttempts == MaxAttempts) break;
  }
  if(WifiConnectAttempts == MaxAttempts)
  {
    lcd.setCursor(0,2);
    lcd.print("Not Connected to AP  ");
    Serial.print("Not Connected to AP  ");
    clientSS = 0;
    RadioName="Not Connected"; 
  }
  else
  {
    GetTime();
    CHour=Hour;
    CMin=Min;
    CSec=Sec;
    lcd.setCursor(0,2);
    lcd.print("Connected to AP     ");
    lcd.setCursor(0,3);
    lcd.print("IP : 192.168.1.249");
    delay(5000); 
    Serial.println("");
    Serial.print("Connected to ");
    if (ssid_temp !="-1")
    {
      Serial.println(ssid_temp); 
    }
    else
    {
      Serial.println(ssid); 
    }
    Serial.println(""); 
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");
    if (SPIFFS.exists("/CRadioName.dat"))
    {
      RadioUrl = LoadDataFromFile("CRadioUrl"); 
      RadioPort = LoadDataFromFile("CRadioPort");
      RadioFile = LoadDataFromFile("CRadioFile");
      RadioName = LoadDataFromFile("CRadioName");
      RadioCity = LoadDataFromFile("CRadioCity");
      RadioCountry = LoadDataFromFile("CRadioCountry");
    }
    else
    {
      RadioUrl = "rss-streaming.co.uk"; 
      RadioPort = "8042";
      RadioFile = "mp3";
      RadioName = "Love80s";
      RadioCity = "Manchester";
      RadioCountry = "United Kingdom";  
    }
    PreviusRadioUrl="rss-streaming.co.uk";
    PreviusRadioPort="8042";
    PreviusRadioFile="mp3";
    PreviusRadioName = "Love80s";
    PreviusRadioCity="Manchester";
    PreviusRadioCountry = "United Kingdom";
    Connect2Radio(RadioUrl, RadioPort, RadioFile, RadioName);
    clientSS = 1;
  }
  TimeLast=millis();
  TimePlay=millis();
}

void loop() 
{
  server.handleClient();
  if (clientSS == 1)
  {
    if (client.available())
    {
      for ( int i=0;i<32;i++)
      {
        Data[i]=client.read();
        delayMicroseconds(1);
      }   
      player.playChunk(Data, 32);
      yield();
      TimePlay=millis();
    }
    else
    {
      if (millis()-TimePlay>3000)
      {
        TimePlay=millis();
        Connect2Radio(RadioUrl, RadioPort, RadioFile, RadioName); 
      }
    }
  }
  else
  {
    if(Alarm == 1)
    {
      TimeNow=millis();
      if (TimeNow - TimeLast >=1000)
      {
        TimeLast = TimeNow;
        CSec +=1;
        if (CSec>=60)
        {
          GetTime();
          CHour=Hour;
          CMin=Min;
          CSec=Sec;
        }
        CenterPrint (3, TimeForm(CHour,CMin,CSec));
        if (CHour==AlarmHour && CMin>=AlarmMin)
        {
          Alarm=0;
          clientSS=0;
          AlarmSong =LoadDataFromFile("AlarmSong");
          if(AlarmSong == "-1") AlarmSong="1";
          AlarmAfter =LoadDataFromFile("AlarmAfter");
          if(AlarmAfter == "-1") AlarmAfter="1";
          if(AlarmAfter != "4")
          {
            for ( int times=0;times<3;times++)
            {
              if(AlarmSong !="5")
              {
                PlayMp3(AlarmSong);
                delay(100);
              }
              else
              {
                for (int SongNum=1;SongNum<5;SongNum++)
                {
                  PlayMp3(String(SongNum, DEC));
                  delay(100);  
                }
              }
            } 
            delay(1000);
            clientSS=1;
            Connect2Radio(RadioUrl, RadioPort, RadioFile, RadioName);
          }
          else
          {
            while(1)
            {
              ////Something to exit from eternity loop?
              if(AlarmSong !="5")
              {
                PlayMp3(AlarmSong);
                delay(100);
              }
              else
              {
                for (int SongNum=1;SongNum<5;SongNum++)
                {
                  PlayMp3(String(SongNum, DEC));
                  delay(100);  
                }
              }  
            }
          }
        }
      }
    }
  }  
}


void Connect2Radio(String RUrl, String RPort , String RFile , String RName)
{
  client.stop();
  client.flush();
  yield();
  delay(100);
  int Trys=0;
  int Trys2=0; 
  while (!client.connect(RUrl.c_str(), RPort.toInt())) 
  {
    delay(100);
    Serial.print(".");
    yield();
    Trys +=1;
    if(Trys > MaxTrys)
    {
      RUrl = PreviusRadioUrl;
      RPort = PreviusRadioPort;
      RFile = PreviusRadioFile;
      RName = PreviusRadioName;
      RadioCity = PreviusRadioCity;
      RadioCountry = PreviusRadioCountry;
      yield();
      if(Trys2 == 1) break;
      Trys=0;
      Trys2 +=1;
    }
  }
  if (Trys < MaxTrys)
  {
    PreviusRadioUrl=RUrl;
    PreviusRadioPort=RPort;
    PreviusRadioFile=RFile;
    PreviusRadioName = RName;
    RadioName = RName;
    PreviusRadioCity = RadioCity;
    PreviusRadioCountry = RadioCountry;
    if (Trys <= 1)
    {
       yield();
       SaveDataToFile("CRadioName", RName);
       SaveDataToFile("CRadioUrl", RUrl);
       SaveDataToFile("CRadioPort", RPort);
       SaveDataToFile("CRadioFile", RFile);
       SaveDataToFile("CRadioCity", RadioCity);
       SaveDataToFile("CRadioCountry", RadioCountry);
    }
    Serial.println("Connected to Host!!!!");
    client.print(String ( "GET /" ) + RFile +
                      String ( " HTTP/1.1\r\n" ) +
                      String ( "Host: " ) + RUrl +
                      String ( "\r\n" ) +
                      String ( "Icy-MetaData:0") + 
                      String ("\r\n" ) +
                      String ( "Connection: close\r\n\r\n" ) ) ;
    yield();
    delay(200);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Now Playing.....");
    CenterPrint (1, RadioName);
    CenterPrint (2, RadioCity);
    CenterPrint (3, RadioCountry);
  }
  else
  {
   yield();
   clientSS=0;
   RadioName="Not Connected :(";
   RadioCity="";
   RadioCountry="";
   lcd.clear();
   CenterPrint (0, "Not Connected :(");                         
  }
}

void handleRoot() 
{
  clientSS=0;
  yield(); 
  HttpString ="<!DOCTYPE html><html><meta charset='UTF-8'>";
  HttpString +="<body><style>a:link, a:visited {background-color: #f44336; color: white; padding: 29px 100px;";
  HttpString +=" width: 760px; align-self: center; justify-content: center; font-size: 100px; text-align: center;";
  HttpString +="text-decoration: none; display: inline-block;} a:hover, a:active { background-color: Violet;}</style>"; 
  HttpString +="<h1 style='font-size:530%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
  if (Alarm !=1)
  {
    HttpString +="<p style= 'text-align:center; font-size:180%; color:blue;'>Now Playing</p>";
    HttpString +="<p style= 'text-align:center; font-size:250%; color:Violet;'>" + RadioName + " " + RadioCity  +"</p>";
    HttpString +="<p style= 'text-align:center; font-size:230%; color:blue;'>" + RadioCountry + "</p>";
  }
  else
  {
    HttpString +="<p style= 'text-align:center; font-size:350%; color:blue;'>Alarm set at :</p>";
    HttpString +="<p style= 'text-align:center; font-size:450%; color:Violet;'>" + AlarmTime+"</p>";
    HttpString +="<p></p>";
  }
  HttpString +="<a href='/Stations'>Stations</a><p></p>";
  HttpString +="<a href='/Alarm'>Alarm</a><p></p>";
  HttpString +="<a href='/Settings'>Settings</a><p></p>";
  HttpString +="<a href='/StartStop'>" + StartStop + "</a>";
  HttpString +="<p style= 'text-align:center; font-size:300%; color:blue;'>" + Bottom + "</p></body></html>"; 
  server.send(200, "text/html",HttpString );
  if (Alarm !=1) clientSS = 1;           
}

void handleAlarm()
{
     clientSS = 0;
     yield(); 
     HttpString ="<!DOCTYPE html><html>";
     HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Set Alarm</h1>";
     HttpString +="<form action='/AlarmSet'><br><input type='time' style='font-size:500%; color:blue' name='alarm_time'>";
     HttpString +="<input type='submit' value=' Set ' style='font-size:450%; float:right; color:red'></form>";
     HttpString +="<p></p><br><br><form method='post' action='/'><p></p><br>";
     HttpString +="<input class='button' type='submit' style='font-size:450%; float:right; color:red' value='Return'></form>";
     server.send(200, "text/html",HttpString );
     if (Alarm !=1) clientSS = 1;
}

void handleAlarmSet()
{
  TempString=server.arg("alarm_time");
  AlarmHour=TempString.substring(0,2).toInt();
  AlarmMin=TempString.substring(3,5).toInt();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alarm set at :");
  CenterPrint (1, TimeForm(AlarmHour,AlarmMin,0));
  GetTime();
  CHour=Hour;
  CMin=Min;
  CSec=Sec;
  clientSS=0;
  AlarmTime=TimeForm(AlarmHour,AlarmMin,0);
  Alarm = 1;
  handleAlarm();
}

void handleAlarmSettings()
{
  clientSS=0;
  yield();
  HttpString ="<!DOCTYPE html><html>";
  HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
  HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Alarm Settings</h1>";
  HttpString +="<form action='/AlarmSettings2'; style='font-size:300%; color:blue'>Alarm Song:<br><br>";
  HttpString +="<select name='AlarmSong' style='font-size:100%; width: 100%; color:Tomato'>";
  HttpString +="<option value='1' selected>Good Morning to you</option><option value='2'>Loud beat</option>";
  HttpString +="<option value='3'>Reveille at West Point</option><option value='4'>Chicken Song</option>";
  HttpString +="<option value='5'>Play all Songs</option></select><p></p>";
  HttpString +="<p style='font-size:100%; color:blue'>After Play Alarm Song :</p>";
  HttpString +="<select name='AlarmAfter' style='font-size:100%; width: 100%; color:Tomato'>";
  HttpString +="<option value='1' selected>Play It 3 Times</option><option value='2'>Play It 5 Times</option>";
  HttpString +="<option value='3'>Play It 10 Times</option><option value='4'>Play It Until Eternity</option></select>";
  HttpString +="<p></p><input type='submit' style='font-size:150%; float:right; color:red'></form>";
  HttpString +="<p></p><br><br><form method='post' action='/'><p></p><br><br><br>";
  HttpString +="<input class='button' type='submit' style='font-size:500%; float:right; color:red' value='Return'></form>";
  server.send(200, "text/html",HttpString );
  clientSS=1;
    
}

void handleAlarmSettings2()
{
  AlarmSong =server.arg("AlarmSong");
  AlarmAfter =server.arg("AlarmAfter");
  SaveDataToFile("AlarmSong", AlarmSong);
  yield();
  delay(100);
  SaveDataToFile("AlarmAfter", AlarmAfter);
  handleSettings();
  
}

void handleSettings()
{
  clientSS=0;
  yield(); 
  HttpString ="<!DOCTYPE html><html><meta charset='UTF-8'>";
  HttpString +="<body><style>a:link, a:visited {background-color: #f44336; color: white; padding: 29px 100px;";
  HttpString +=" width: 760px; align-self: center; justify-content: center; font-size: 100px; text-align: center;";
  HttpString +="text-decoration: none; display: inline-block;} a:hover, a:active { background-color: Violet;}</style>"; 
  HttpString +="<h1 style='font-size:530%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
  HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Settings</h1><br><br>";
  HttpString +="<a href='/Wifi'>Wifi Settings</a><p></p>";
  HttpString +="<a href='/Sound'> Sound Settings</a><p></p>";
  HttpString +="<a href='/AlarmSettings'> Alarm Settings</a><p></p>";
  HttpString +="<a href='/'>Return</a><p></p><br><br><br><br>";
  HttpString +="<p style= 'text-align:center; font-size:320%; color:blue;'>" + Bottom + "</p></body></html>";
  server.send(200, "text/html",HttpString );
  clientSS=1;
}


void handleStartStop()
{
  if (StartStop == "Start")
  {
    StartStop = "Stop";
    player.setVolume(VOLUME.toInt());
  }
  else
  {
    StartStop = "Start";
    player.setVolume(0);
  }
  handleRoot();  
}

void handleStations() 
{
     clientSS=0;
     yield(); 
     HttpString ="<!DOCTYPE html><html>";
     HttpString +="<body><style>a:link, a:visited {background-color: #f44336; color: white; padding: 30px 100px;";
     HttpString +=" width: 760px; align-self: center; justify-content: center; font-size: 100px; text-align: center;";
     HttpString +="text-decoration: none; display: inline-block;} a:hover, a:active { background-color: Violet;}</style>"; 
     HttpString +="<h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
     HttpString +="<a href='/PeekStation'>Peek a Station</a><p></p><br>";
     HttpString +="<a href='/NewStation'>New Station</a><p></p><br>";
     HttpString +="<a href='/MonitorStation'>Monitor Station</a><p></p><br>";
     HttpString +="<a href='/'>Return</a><p></p><br><br><br><br><br>";
     HttpString +="<p style= 'text-align:center; font-size:320%; color:blue;'>" + Bottom + "</p></body></html>"; 
     server.send(200, "text/html",HttpString );
     clientSS=1;
}

void handleMonitorStation()
{
     yield();
     HttpString ="<!DOCTYPE html><html>";
     HttpString +="<body style='background-color:LightBlue'><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Monitor Station</h1><br><br>";
     HttpString +="<br><br><audio controls autoplay='True' preload='auto' style='width:700px;'>";
     HttpString +="<source src='http://"+RadioUrl+":"+RadioPort+"/"+RadioFile+"' type='audio/mpeg'>";
     HttpString +="Your browser does not support the audio element.</audio><br><br><br>";
     HttpString +="<br><br><br><form method='post' action='/Stations'><p></p><br><br><br><br><br><br>";
     HttpString +="<input class='button' type='submit' style='font-size:450%; float:right; color:red' value='Return'></form>";
     HttpString +="<br><br><br><br><br><br><p style= 'text-align:center; font-size:320%; color:blue;'>" + Bottom + "</p></body></html>"; 
     server.send(200, "text/html",HttpString );
}

void handleNewStation()
{
     clientSS=0;
     yield(); 
     HttpString ="<!DOCTYPE html><html>";
     HttpString +="<body style='background-color:LightBlue'><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Test Station</h1><br><br>";
     HttpString +="<form action='/TestStation'; style='font-size:500%; color:Tomato'>Station URL:<br>";
     HttpString +="<input type='text' name='stationurl' style='font-size:100%; color:blue' value='"+RadioUrl+"'><br>Station Port:";
     HttpString +="<input type='text' name='stationport' style='font-size:100%; color:blue' value='"+RadioPort+"'><br>Station File:";
     HttpString +="<input type='text' name='stationfile' style='font-size:100%; color:blue' value='"+RadioFile+"'><br><br>";
     HttpString +="<input type='submit' style='font-size:100%; color:red' value='Test'></form><br><br><br>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Save Station</h1><br><br>";
     HttpString +="<form action='/SaveStation' style='font-size:500%; color:Tomato'>Station URL:<br>";
     HttpString +="<input type='text' name='stationurl' style='font-size:100%; color:blue'><br>Station Port:";
     HttpString +="<input type='text' name='stationport' style='font-size:100%; color:blue'><br>Station File:";
     HttpString +="<input type='text' name='stationfile' style='font-size:100%; color:blue'><br>";
     HttpString +="Station Name:<br><input type='text' name='stationame' style='font-size:100%; color:blue'><br>Station Genre:<br>";
     HttpString +="<input type='text' name='stationgerne' style='font-size:100%; color:blue'><br>Station Country:<br>";
     HttpString +="<input type='text' name='stationcountry' style='font-size:100%; color:blue'><br>Station City:<br>";
     HttpString +="<input type='text' name='stationcity' style='font-size:100%; color:blue'><br><br>";
     HttpString +="<input type='submit' style='font-size:100%; color:red' value='Save'></form><br><br><br>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Upload Stations Files</h1><br><br>";
     HttpString +="<form method='post' action='/FileUpload' enctype='multipart/form-data'> <input type='file' style='font-size:450% color:blue' name='Upload'><br><br>";
     HttpString +="<input class='button' type='submit' style='font-size:450%; float:right; color:red' value='Upload'></form>";
     HttpString +="<form method='post' action='/'><p></p><br><br><br><br><br><br>";
     HttpString +="<input class='button' type='submit' style='font-size:450%; float:right; color:red' value='Return'></form>";
     server.send(200, "text/html", HttpString );
     clientSS=1;
}

void handleTestStation()
{
  RadioUrl=server.arg("stationurl");
  RadioPort=server.arg("stationport");
  RadioFile=server.arg("stationfile");
  Connect2Radio(RadioUrl, RadioPort, RadioFile, "Test");
  handleNewStation();
}

void handleSaveStation()
{
  RadioUrl=server.arg("stationurl");
  RadioPort=server.arg("stationport");
  RadioFile=server.arg("stationfile");
  RadioCountry=server.arg("stationcountry");
  RadioCity=server.arg("stationcity");
  RadioName=server.arg("stationame");
  RadioGenre=server.arg("stationgerne");
  RadioNum="";
  TempString="";
  //////////////////////////////////////////////////////////
     
}

void handleFileUpload()
{
   clientSS = 0;
   HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w"); 
    filename = String();
  } 
  else if(upload.status == UPLOAD_FILE_WRITE)
  {
    Serial.println(upload.currentSize);
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
  } 
  else if(upload.status == UPLOAD_FILE_END)
  {
    if(fsUploadFile) 
    {                                    
      fsUploadFile.close();                              
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");
      server.send(303);
    } 
    else 
    {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
  clientSS = 1;
}



void handlePeekStation()
{
     clientSS=0;
     client.stop();
     yield(); 
     HttpString ="<!DOCTYPE html><html>";
     HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Peek a Station by:</h1>";
     HttpString +="<form action='/ViewStations'><select name='genre' style='font-size:500%; width: 100%; color:Tomato'>"; 
     HttpString +=AllGenre;
     HttpString +="</select><p></p><br><select name='country' style= 'font-size:500%; width: 100%; color:Tomato'>";
     HttpString +=AllCountry;
     HttpString +="</select><p></p><br><select name='city' style= 'font-size:500%; width: 100%; color:Tomato'>";
     HttpString +=AllCity;
     HttpString +="</select><p></p><br><br><br><br><br><input type='submit' style= 'font-size:500%; float: right; color:Tomato'></form><br><br><br>";
     HttpString +="<br><br><br><br><p style= 'text-align:center; font-size:320%; color:blue;'>" + Bottom + "</p></body></html>";
     server.send(200, "text/html", HttpString );
     Connect2Radio(RadioUrl, RadioPort, RadioFile, RadioName);
     clientSS=1;
}

void handleViewStation()
{ 
   clientSS = 0;
   client.flush();
   client.stop();
   yield();
   String Curgenre=server.arg("genre");
   String Curcountry=server.arg("country");
   String Curcity=server.arg("city");
   File f = SPIFFS.open("/stations.dat", "r");
   if (f)
   {
       int offset=0;
       TempString4="";
       while (offset<=f.size())
       {
         f.seek(offset, SeekSet);
         TempString = f.readStringUntil('\r');
         bool Ge  = Curgenre.substring(2,Curgenre.length()) == TempString.substring(StringFind (TempString,"^")+1,StringFind (TempString,"&")); 
         bool Co = Curcountry.substring(0,2) == TempString.substring(StringFind (TempString,"#")+1,StringFind (TempString,"$"));
         Co = Co || Curcountry.substring(2,4) == TempString.substring(StringFind (TempString,"#")+1,StringFind (TempString,"$"));  
         bool Ci = Curcity.substring(2,Curcity.length())  == TempString.substring(StringFind (TempString,"$")+1,StringFind (TempString,"%"));
         if((Ge  || Co  || Ci) && !TempString.startsWith("[") )
         {
           TempString2=TempString.substring(StringFind (TempString,"%")+1,StringFind (TempString,"^"));
           TempString4 += "<option value='" + TempString.substring(0,5) + "'>" + TempString2 + "</option>";
         }
         offset += TempString.length()+ 1;
         yield();
       }
       
   }
   f.close();
   HttpString ="<!DOCTYPE html><html>";
   HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
   HttpString +="<p style= 'text-align:center; font-size:320%; color:blue;'>Stations</p>";
   HttpString +="<form action='/StationChoice'><p></p>";
   HttpString +="<select name='Station' style='font-size:500%; width: 100%; color:Tomato'>"; 
   HttpString +=TempString4;
   HttpString +="</select><p></p><br><input type='submit' style= 'font-size:500%; float: right; color:Tomato'><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br></form>";
   HttpString +="<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>";
   HttpString +="<p style= 'text-align:center; font-size:320%; color:blue;'>" + Bottom + "</p></body></html>";
   server.send(200, "text/html", HttpString );
   Connect2Radio(RadioUrl, RadioPort, RadioFile, RadioName);
   clientSS = 1;
}

void handleStationChoise()
{
  client.flush();
  client.stop();
  yield(); 
  clientSS = 0;
  CRadioNum=server.arg("Station");
  Serial.println("Station Number : "+CRadioNum);
  File f = SPIFFS.open("/stations.dat", "r");
  if (f)
  {
       int offset=0;
       while (offset<=f.size())
       {
         f.seek(offset, SeekSet);
         TempString = f.readStringUntil('\r');
         RadioNum = TempString.substring(0,5);
         if (CRadioNum.toInt() == RadioNum.toInt())
         {
           RadioUrl = TempString.substring(StringFind (TempString,"*")+1,StringFind (TempString,"!"));
           RadioPort = TempString.substring(StringFind (TempString,"!")+1,StringFind (TempString,"@"));
           RadioFile = TempString.substring(StringFind (TempString,"@")+1,StringFind (TempString,"#"));
           RadioFile.replace("~", "%");
           RadioFile.replace("+", "&");
           Serial.println(RadioFile);
           RadioName = TempString.substring(StringFind (TempString,"%")+1,StringFind (TempString,"^"));
           RadioCity = TempString.substring(StringFind (TempString,"$")+1,StringFind (TempString,"%"));
           RadioCountry = TempString.substring(StringFind (TempString,"#")+1,StringFind (TempString,"$"));
           Serial.println(RadioUrl);
           Serial.println(RadioCountry);
           break; 
         }
         offset += TempString.length()+ 1;
       }
  }
  f.close();
  File f1 = SPIFFS.open("/countries.dat", "r");
  if (f1)
  {
       int offset=0;
       while (offset<=f1.size())
       {
         f1.seek(offset, SeekSet);
         TempString = f1.readStringUntil('\r');
         if(StringFind (TempString, RadioCountry) != -1)
         {
           RadioCountry = TempString.substring(4,TempString.length());
           break;
         }
         offset += TempString.length()+ 1;
       }
  }
  f1.close();
  HttpString ="<!DOCTYPE html><html>";
  HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
  HttpString +="<br><br><br><br><p style= 'text-align:center; font-size:450%; color:red;'>" + RadioName + "</p>";
  HttpString +="<p style= 'text-align:center; font-size:450%; color:blue;'>" + RadioCity + "</p>";
  HttpString +="<form action='/'><p></p><br><br><input type='submit' style= 'font-size:500%; float: right; color:Tomato'></form><br><br><br>";
  HttpString +="<br><br><br><br><br><br><br><br><br><br><br><br><br><br><p style= 'text-align:center; font-size:320%; color:blue;'>" + Bottom + "</p></body></html>";
  server.send(200, "text/html", HttpString );
  Connect2Radio(RadioUrl, RadioPort, RadioFile, RadioName);
  clientSS = 1;     
}

String OptionAdd(String FileName)
{
   String TString="";
   File f = SPIFFS.open("/" + FileName + ".dat", "r");
   if (f)
   {
       int offset=0;
       while (offset<=f.size())
       {
         f.seek(offset, SeekSet);
         TempString = f.readStringUntil('\r');
         TString += "<option value='" + TempString + "'>" + TempString + "</option>";
         offset += TempString.length()+ 1;
       }
   }
   else
   {
       TString += "<option value='Error'>Error</option>"; 
   }
   f.close();
   return TString;
    
}


void handleSound() 
{
     clientSS = 0;
     yield(); 
     HttpString ="<!DOCTYPE html><html>";
     HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Sound Settings</h1>";
     HttpString +="<h1 style='font-size:300%; color:blue;'>Volume Settings</h1>";
     HttpString +="<form action='/Volume' style='font-size:400%; color:blue' oninput='x.value=parseInt(r1.value)'>Volume : 0";
     HttpString +="<input type='range' id='r1' name='r1' value='80' style='font-size:100%'>100 : &nbsp; <output name='x' for='r1'>&nbsp;</output>";
     HttpString +="<input style='font-size:100%; color:blue' type='submit' value='Save'></form>";
     HttpString +="<h1 style='font-size:300%; color:blue;'>Bass Settings</h1>";
     HttpString +="<form action='/Bass' style='font-size:400%; color:blue'>Boost dB (0-15) : ";
     HttpString +="<input type='number' style='font-size:100%; color:blue' id='r2' name='r2' min='0' max='15'";
     HttpString +="value='" + String(BassBoost) + "'><br>Freq Hz (20-150) : ";
     HttpString +="<input type='number' style='font-size:100%; color:blue' id='r3' name='r3' min='20' max='150' value='" + String(BassFreq) + "'>";
     HttpString +="<input style='font-size:100%; color:blue' type='submit' value='Save'></form><br>";
     HttpString +="<h1 style='font-size:300%; color:blue;'>Treble Settings</h1>";
     HttpString +="<form action='/Treble' style='font-size:400%; color:blue'>Boost dB (-8  - 7) : ";
     HttpString +="<input type='number' style='font-size:100%; color:blue' id='r4' name='r4' min='-8' max='7' value='" + String(TrebleBoost) + "'><br>Freq (1-15)KHz :";
     HttpString +="<input type='number' style='font-size:100%; color:blue' id='r5' name='r5' min='1' max='15' value='" + String(TrebleFreq) + "'>";
     HttpString +="<input style='font-size:100%; color:blue' type='submit' value='Save'></form><br>";
     HttpString +="<form method='post' action='/Settings'><p></p><br><br>";
     HttpString +="<input class='button' type='submit' style='font-size:450%; float:right; color:red' value='Return'></form>";
     server.send(200, "text/html",HttpString );
     clientSS = 1;
}

void handleVolume()
{
  VOLUME=server.arg("r1");
  Serial.println(VOLUME);
  player.setVolume(VOLUME.toInt());
  SaveDataToFile("VOL", VOLUME);
  handleSound();
  
}

void handleBass()
{
  BassBoost=server.arg("r2").toInt();
  BassFreq=server.arg("r3").toInt();
  SetTone();
  handleSound();
  
}

void handleTreble()
{
  TrebleBoost=server.arg("r4").toInt();
  TrebleFreq=server.arg("r5").toInt();
  SetTone();
  handleSound(); 
}

void SetTone()
{
  Tone[0]=TrebleBoost;
  Tone[1]=TrebleFreq;
  Tone[2]=BassBoost;
  Tone[3]=BassFreq/10;
  player.setTone(Tone);
  SaveDataToFile("TrebleBoost", String(TrebleBoost));
  SaveDataToFile("TrebleFreq", String(TrebleFreq));
  SaveDataToFile("BassBoost", String(BassBoost));
  SaveDataToFile("BassFreq", String(BassFreq));
}

void handleWifi() 
{
     clientSS = 0;
     client.flush();
     client.stop();
     yield();
     int n = WiFi.scanNetworks();
     Serial.println(n);
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Found "+String(n)+" Networks");
     TempString="";
     TempString="<ol style='list-style-type:disc; color:blue; font-size:200%'>";
     if (n == 0 )
     {
       TempString +="<li>None</li>";
     }
     else
     {
       for (int i=0 ; i<n ; i++)
       {
        TempString +="<li>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")" + "</li>";
        delay(10);  
       }
     }
     TempString +="</ol>";
     HttpString ="<!DOCTYPE html><html>";
     HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
     HttpString +="<h1 style='font-size:450%; text-align:center; color:red;'>Wifi Settings</h1>";
     HttpString +="<p style='font-size:300%;'>Available Access Point :</p>";
     HttpString +=TempString;
     HttpString +="<form action='/APSettings'; style='font-size:300%; color:red;';>Access Point SSID:<br><input style='font-size:100%' type='text' name='APSSID' value='Your ssid'><br>";
     HttpString +="Access Point Password:<br><input style='font-size:100%' type='password' name='APPass'>";
     HttpString +="<input style='font-size:100%' type='submit' value='Save'></form>";
     HttpString +="<p style='font-size:350%;'>Station Settings</p>";
     HttpString +="<form action='/StSettings'; style='font-size:300%; color:red;';>Station SSID:<br><input style='font-size:100%' type='text' name='StSSID' value='InternetRadio'><br>";
     HttpString +="Station Password:<br><input style='font-size:100%' type='text' name='StPass' value='123445767890'>";
     HttpString +="<input style='font-size:100%' type='submit' value='Save'></form><br><br><br>";
     HttpString +="<form method='post' action='/Settings'><p></p><br><br><br>";
     HttpString +="<input class='button' type='submit' style='font-size:450%; float:right; color:red' value='Return'></form>";
     server.send(200, "text/html",HttpString );
     Connect2Radio(RadioUrl, RadioPort, RadioFile, RadioName);
     clientSS = 1;
}

void handleAPSettings() 
{
  ssid_temp=server.arg("APSSID");
  password_temp=server.arg("APPass");
  SaveDataToFile("APSSID", ssid_temp);
  SaveDataToFile("APPass", password_temp);
  HttpString ="<!DOCTYPE html><html>";
  HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
  if(TempString !="-1")
  {
    HttpString +="<p style='font-size:300%';>Acces Point name : " + ssid_temp + " and Password : " + password_temp + " Saved successfully!</p>";
    HttpString +="<p style='font-size:300%';>System will reboot in 3 seconds</p>"; 
  }
  else
  {
    HttpString +="<p style='font-size:300%';>Error in save procces!!</p>";
     
  }
  server.send(200, "text/html", HttpString );
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting to...");
  lcd.setCursor(0,1);
  lcd.print(ssid_temp);
  delay(3000);
  if(TempString !="-1") ESP.restart(); 
}

void handleStSettings() 
{
  ssid_temp=server.arg("StSSID");
  password_temp=server.arg("StPass");
  SaveDataToFile("StSSID", ssid_temp);
  SaveDataToFile("StPass", password_temp);
  Serial.println(TempString);
  HttpString ="<!DOCTYPE html><html>";
  HttpString +="<body><h1 style='font-size:550%; text-align:center; background-color:blue; color:white;'>" + Version + "</h1>";
  if(TempString !="-1")
  {
     HttpString +="<p style='font-size:300%'; color:blue>Station name : " + ssid_temp + " and Password : " + password_temp + " Saved successfully!</p>";
     HttpString +="<p style='font-size:300%';>System will reboot in 3 seconds</p>"; 
  }
  else
  {
     HttpString +="<p style='font-size:300%';>Error in save procces!!</p>"; 
  }
  server.send(200, "text/html", HttpString);
  delay(3000);
  if(TempString !="-1") ESP.reset();
}

int StringFind (String StringToSearch, String StringToFind)
{
  int StringPos=-1; 
  for (int i=0;i<=StringToSearch.length()-StringToFind.length();i++)
  {
      if (StringToSearch.substring(i,i+StringToFind.length())==StringToFind)
      {
       StringPos=i;
       break; 
      }
   }
   return StringPos;    
}

String StringRemove(String StringToSearch, String StringToRemove)
{
   int StringIndex = StringToSearch.indexOf(StringToRemove);
   if(StringIndex != -1)
   {
      StringToSearch.remove(StringIndex, StringToRemove.length());
   }
   return StringToSearch;
}

void SaveDataToFile(String fileNameForSave, String DataToSave)
{
  File f = SPIFFS.open(String("/" + fileNameForSave + ".dat"), "w");
  if (!f)
  {
    TempString="-1";
  }
  else
  {
    f.println(String(DataToSave + String('\r')));
    f.close();
    TempString="0";
  }
  return;
}

String LoadDataFromFile(String fileNameForSave)
{
  String WhatIwillReturn;
  File f = SPIFFS.open(String("/" + fileNameForSave + ".dat"), "r");
  if (!f)
  {
    WhatIwillReturn ="-1";
  }
  else
  {
    WhatIwillReturn =  f.readStringUntil('\r');
    WhatIwillReturn.replace("\n", "");
  }
  f.close();
  return WhatIwillReturn;
}

void ClsRow(byte row)
{
  lcd.setCursor(0,row);
  lcd.print("                    "); //Lcd 20x4
}

void CenterPrint (byte row , String text)
{
  ClsRow(row);
  byte col=(20-text.length())/2;  //Lcd 20x4
  lcd.setCursor(col,row);
  lcd.print(text);
}

void GetTime()
{
  client.stop();
  yield();
  while (!client.connect(TimeServer.c_str(),80))
  {
   delay(100);
   Serial.print(".");
   yield();  
  }
  Serial.println("Connected!!!");
  client.println("GET/ HTTP/1.1\r\n" );
  delay(500);
  while (client.available())
  {
    TempString=client.readStringUntil('\r');
    if(StringFind(TempString,"Date:") !=-1)
    {
      DateString=TempString.substring(7, 23);
      TimeString=TempString.substring(24,32);
      Hour=TimeString.substring(0, 2).toInt();
      Hour +=3;
      if(Hour>=24) Hour = Hour-24;
      Min=TimeString.substring(3, 5).toInt();
      Sec=TimeString.substring(6, 8).toInt();
      Serial.println(Hour);
      Serial.println(Min);
      Serial.println(Sec);
      break; 
    }  
  }
  client.flush();
  client.stop();
  yield();
}

String TimeForm( int Inhour, int Inmin, int Insec)
{
   String WhatIwillReturn;
   if (Inhour<10)
   {
     WhatIwillReturn="0"+String(Inhour)+":"; 
   }
   else
   {
     WhatIwillReturn=String(Inhour)+":";
   }
   if (Inmin<10)
   {
     WhatIwillReturn +="0"+String(Inmin)+":"; 
   }
   else
   {
     WhatIwillReturn +=String(Inmin)+":";
   }
   if (Insec<10)
   {
     WhatIwillReturn +="0"+String(Insec); 
   }
   else
   {
     WhatIwillReturn +=String(Insec);
   }
   return WhatIwillReturn;
}

void PlayMp3(String What2Play)
{
   unsigned char Mp3Data[32] , Bytes2Write=32;
   File f = SPIFFS.open(String("/" + What2Play + ".mp3"), "r");
   if (f)
   {
     unsigned long offset=256;
     while (offset<f.size())
     {
       f.seek(offset, SeekSet);
       if (f.size()-32<offset) Bytes2Write=f.size()-offset;
       for ( int i=0;i<Bytes2Write;i++)
       {
        Mp3Data[i]=f.read();
        delayMicroseconds(1);
       }
       player.playChunk(Mp3Data, Bytes2Write);
       yield();
       offset += Bytes2Write;
     }
   }
   f.close();
}

