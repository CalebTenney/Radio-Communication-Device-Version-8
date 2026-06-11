#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "time.h"
#include <esp_now.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C  //if not working, channge to 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
uint8_t broadcastAddress[] = { 0xCC, 0xDB, 0xA7, 0x03, 0x00, 0xB8 };
char* ssid = "DEFAULT SSID #1";
char* password = "DEFAULT SSID #1 PASSWORD";
char* ssidII = "DEFAULT SSID #2";
char* passwordII = "DEFAULT SSID #2 PASSWORD";
char ssidIII[32];
char passwordIII[32];
char MorseChar[4];
char Seconds[3];
char Minutes[3];
char Hours[3];
char Days[3];
char Months[4];
int LU = 26;
int LD = 14;
int MU = 27;
int MD = 12;
int RU = 25;
int RD = 13;
int Time = 0;
int microseconds = 0;
int prevmicros = 0;
int hourmicros = 0;
int Bcheck = 0;
int result = 0;
int prevresult = 0;
int Battery = 16;
int Charging = 0;
int Display = 1;
int Wcycle = 0;
int Wf = 1;
int Mode = 0;
int settsmode = 0;
int Connected = 0;
int wifitry = 0;
int wifitryII = 0;
int Getime = 1;
int Getweather = 0;
int Temp = 0;
int Wind = 0;
int Humidity = 0;
int Brightness = 127;
int Screenoff = 20;
int NewMessage = 0;
int Unread = 0;
int Scroll = 0;
int TopScroll = 0;
int address = 0;
int SendStatus = 2;
int num = 0;
int cur = 0;
int passcur = 0;
int netcount = 0;
int netcycle = 0;
int nscycle = 0;
int nety = 12;
int StartMess = 0;
int Knownet = 0;
String GOOGLE_SCRIPT_ID = "INSERT ID HERE";
String Mtext = "     ";
String jsonBuffer;
struct tm timeinfo;
struct struct_messages {
  char text[32];
  char id[7];
} Message, RMessage, RMessageII, RMessageIII, RMessageIV, RMessageV, RMessageVI;
esp_now_peer_info_t peerInfo;
void setup() {
  pinMode(LU, INPUT_PULLUP);
  pinMode(LD, INPUT_PULLUP);
  pinMode(MU, INPUT_PULLUP);
  pinMode(MD, INPUT_PULLUP);
  pinMode(RU, INPUT_PULLUP);
  pinMode(RD, INPUT_PULLUP);
  Message.id[0] = 'C';
  Message.id[1] = 'a';
  Message.id[2] = 'l';
  Message.id[3] = 'e';
  Message.id[4] = 'b';
  EEPROM.begin(400);
  Brightness = EEPROM.read(0);
  Screenoff = EEPROM.read(1);
  EEPROM.get(2, RMessage);
  EEPROM.get(42, RMessageII);
  EEPROM.get(82, RMessageIII);
  EEPROM.get(122, RMessageIV);
  EEPROM.get(162, RMessageV);
  EEPROM.get(202, RMessageVI);
  EEPROM.get(242, ssidIII);
  EEPROM.get(275, passwordIII);
  /*
   * 0: Brightness Level
   * 1: Screen Timeout
   * 2-42: RMessage
   * 42-82: RMessageII
   * 82-122: RMessageIII
   * 122-162: RMessageIV
   * 162-202: RMessageV
   * 202-242: RMessageVI
   * 242-275: ssidIII
   * 275-308: passwordIII
   */
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
}

void loop() {
  TimeKeeping();
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(Brightness * 2);
  //if((Getime==0)&&(Display==1))BatteryLevel();
  if (WiFi.status() == WL_CONNECTED) Connected = 1;
  else Connected = 0;
  if ((digitalRead(LU) == 1) && (digitalRead(LD) == 1) && (digitalRead(MU) == 1) && (digitalRead(MD) == 1) && (digitalRead(RU) == 1) && (digitalRead(RD) == 1)) Bcheck = 0;
  else {
    if (Display == 0) {
      Display = 1;
      Bcheck = 1;
    }
    if (NewMessage != 0) {
      NewMessage = 0;
      Bcheck = 1;
    }
    Time = microseconds;
  }
  if (Getweather == 1) {
    if (Connected == 1) {
      jsonBuffer = httpGETRequest("http://api.openweathermap.org/data/2.5/weather?q=Laurel&APPID=90b6656fc24e0344c91180539d77f955&units=imperial");
      JSONVar myObject = JSON.parse(jsonBuffer);
      Temp = myObject["main"]["temp"];
      Humidity = myObject["main"]["humidity"];
      Wind = myObject["wind"]["speed"];
      WiFi.disconnect(true);
      Getweather = 0;
      EspNowPrep();
    }
  }
  if (NewMessage > 0) {
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(RMessage.id);
    display.print(":");
    display.setCursor(0, 16);
    display.print(RMessage.text);
    NewMessage++;
    Time = microseconds;
    if (NewMessage > 150) {
      NewMessage = 0;
      Display = 1;
    }
  }
  if ((Display == 1) && (NewMessage == 0)) {
    if (Wf == 1) {
      if (((digitalRead(LD) == 0) or (digitalRead(MU) == 0) or (digitalRead(MD) == 0) or (digitalRead(RU) == 0) or (digitalRead(RD) == 0)) && (Bcheck == 0)) {
        Wf = 0;
        display.clearDisplay();
        Bcheck = 1;
      }
      display.setTextSize(4);
      display.setCursor(0, 17);
      display.print(Hours);
      display.print(":");
      display.print(Minutes);
      display.setTextSize(2);
      display.setCursor(54, 50);
      display.print(Seconds);
      /*if(Charging==1)display.fillRoundRect(108, 3, 16, 7, 2, SSD1306_BLACK);
display.drawRoundRect(106, 1, 20, 11, 3, SSD1306_WHITE);
display.fillRoundRect(108, 3, Battery, 7, 2, SSD1306_WHITE);
display.drawLine(127,4,127,7,SSD1306_WHITE);
if(Charging==1){
  display.drawLine(111,7,117,7,SSD1306_INVERSE);
  display.drawLine(116,6,115,6,SSD1306_INVERSE);
  display.drawLine(114,5,120,5,SSD1306_INVERSE);
}*/
      if (StartMess > 1) {
        display.setTextSize(2);
        display.setCursor(0, 0);
        if (Knownet == 1) {
          if (StartMess <= 35) display.print("Could Not");
          if ((StartMess > 35) && (StartMess <= 70)) display.print("Connect.");
          if (StartMess > 70) display.print("Restart?");
          if (digitalRead(MU) == 0) {
            display.clearDisplay();
            StartMess = 70;
            Restart();
          }
        } else {
          if (StartMess <= 35) display.print("No Known");
          if ((StartMess > 35) && (StartMess <= 70)) display.print("Networks.");
          if (StartMess > 70) display.print("Show Avail");
          if (digitalRead(MU) == 0) {
            Bcheck = 1;
            Mode = 1;
            settsmode = 1;
          }
        }
        if (StartMess > 140) StartMess = 2;
        StartMess++;
      }
      if (Unread > 0) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(Unread);
        display.setTextSize(2);
        display.print(RMessage.id[0]);
        display.print(":");
        address = TopScroll;
        while (address < TopScroll + 7) {
          if (address < 31) display.print(RMessage.text[address]);
          address++;
        }
        TopScroll++;
        if (TopScroll > 30) TopScroll = 0;
        delay(200);
      }
      if ((Unread == 0) && (StartMess == 0)) {
        if (Connected == 1) WiFiSymbol();
        if (Wcycle == 0) {
          if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
            Wcycle = 1;
            Bcheck = 1;
          }
          display.setCursor(33, 0);
          display.print(Months);
          display.setCursor(73, 0);
          display.print(Days);
        }
        if (Wcycle == 1) {
          if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
            Wcycle = 0;
            Bcheck = 1;
          }
          if (digitalRead(MU) == 0) {
            Wf = 0;
            Mode = 2;
            Bcheck = 1;
          }
          display.setCursor(54, 0);
          display.print(Temp);
          display.setTextSize(1);
          display.setCursor(80, 0);
          display.print("o");
        }
      }
    }
    if (Wf == 0) {
      if (Mode == 0) {
        display.fillTriangle(5, 13, 20, 5, 20, 21, SSD1306_WHITE);
        display.fillRoundRect(20, 12, 13, 4, 1, SSD1306_WHITE);
        display.fillCircle(95, 15, 8, SSD1306_WHITE);
        display.fillCircle(105, 13, 11, SSD1306_WHITE);
        display.fillCircle(115, 15, 8, SSD1306_WHITE);
        display.drawLine(47, 8, 75, 8, SSD1306_WHITE);
        display.fillCircle(59, 8, 2, SSD1306_WHITE);
        display.drawLine(47, 14, 75, 14, SSD1306_WHITE);
        display.fillCircle(52, 14, 2, SSD1306_WHITE);
        display.drawLine(47, 20, 75, 20, SSD1306_WHITE);
        display.fillCircle(63, 20, 2, SSD1306_WHITE);
        display.fillCircle(14, 43, 12, SSD1306_WHITE);
        display.fillCircle(25, 43, 12, SSD1306_WHITE);
        display.drawLine(14, 30, 25, 30, SSD1306_WHITE);
        display.drawLine(14, 31, 25, 31, SSD1306_WHITE);
        display.drawLine(14, 56, 25, 56, SSD1306_WHITE);
        display.drawLine(14, 55, 25, 55, SSD1306_WHITE);
        display.fillTriangle(13, 55, 5, 51, 2, 58, SSD1306_WHITE);
        if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
          Wf = 1;
          Bcheck = 1;
        }
        if ((digitalRead(MU) == 0) && (Bcheck == 0)) {
          Mode = 1;
          Bcheck = 1;
        }
        if ((digitalRead(RU) == 0) && (Bcheck == 0)) {
          Mode = 2;
          Bcheck = 1;
        }
        if ((digitalRead(LD) == 0) && (Bcheck == 0)) {
          Mode = 4;
          Bcheck = 1;
        }
      }
      if (Mode == 1) {
        if (settsmode < 5) {
          display.setTextSize(1);
          display.setCursor(40, 0);
          display.print("Settings");
          display.setTextSize(2);
          display.setCursor(2, 12);
          display.print("Display");
          display.setCursor(2, 32);
          display.print("WiFi");
          if (settsmode == 0) {
            display.fillRect(0, 10, 127, 20, SSD1306_INVERSE);
            if ((digitalRead(LD) == 0) && (Bcheck == 0)) {
              settsmode = 5;
              Bcheck = 1;
            }
          }
          if (settsmode == 1) {
            display.fillRect(0, 30, 127, 20, SSD1306_INVERSE);
            if (((digitalRead(LD) == 0) && (Bcheck == 0)) or (Knownet == 0)) {
              display.clearDisplay();
              display.setCursor(0, 15);
              display.print("Getting   Networks..");
              display.display();
              WiFi.mode(WIFI_STA);
              WiFi.disconnect(true);
              netcount = WiFi.scanNetworks();
              netcycle = 0;
              settsmode = 6;
              Knownet = 1;
              Bcheck = 1;
            }
          }
          if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
            Mode = 0;
            Bcheck = 1;
          }
          if ((digitalRead(RU) == 0) && (Bcheck == 0)) {
            settsmode--;
            if (settsmode < 0) settsmode = 1;
            Bcheck = 1;
          }
          if ((digitalRead(RD) == 0) && (Bcheck == 0)) {
            settsmode++;
            if (settsmode > 1) settsmode = 0;
            Bcheck = 1;
          }
        }
        if (settsmode == 5) {
          if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
            settsmode = 0;
            Bcheck = 1;
            EEPROM.write(0, Brightness);
            EEPROM.write(1, Screenoff);
            EEPROM.commit();
          }
          if ((digitalRead(RU) == 0) && (Bcheck == 0)) {
            Screenoff = Screenoff + 5;
            Bcheck = 1;
          }
          if ((digitalRead(RD) == 0) && (Bcheck == 0)) {
            if (Screenoff == 61) Screenoff = 60;
            else Screenoff = Screenoff - 5;
            Bcheck = 1;
          }
          if (Screenoff < 5) Screenoff = 5;
          if (Screenoff > 61) Screenoff = 61;
          if (digitalRead(LD) == 0) Brightness--;
          if (digitalRead(MD) == 0) Brightness++;
          if (Brightness > 127) Brightness = 127;
          if (Brightness < 1) Brightness = 1;
          display.drawRoundRect(0, 5, 127, 20, 5, SSD1306_WHITE);
          display.fillRoundRect(0, 5, Brightness, 20, 5, SSD1306_WHITE);
          display.setTextSize(1);
          display.setCursor(0, 26);
          display.print("Brightness");
          display.setCursor(0, 41);
          display.print("Screen Timeout");
          display.setTextSize(2);
          if (Screenoff == 61) {
            display.setCursor(5, 50);
            display.print("Never");
          } else {
            display.setCursor(5, 50);
            display.print(Screenoff);
            display.print(" secs");
          }
        }
        if ((settsmode > 5) && (settsmode < 11)) {
          if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
            settsmode = 1;
            Bcheck = 1;
          }
          if (netcount == 0) {
            display.setCursor(2, 12);
            display.print("No networks found");
          } else {
            if ((digitalRead(RU) == 0) && (Bcheck == 0)) {
              settsmode--;
              if ((settsmode < 6) && (netcount >= 5)) settsmode = 10;
              if ((settsmode < 6) && (netcount < 5)) settsmode = netcount + 5;
              Bcheck = 1;
            }
            if ((digitalRead(RD) == 0) && (Bcheck == 0)) {
              settsmode++;
              if ((settsmode > 10) && (netcount >= 5)) settsmode = 6;
              if ((settsmode > netcount + 5) && (netcount < 5)) settsmode = 6;
              Bcheck = 1;
            }
            if ((digitalRead(LD) == 0) && (Bcheck == 0)) {
              display.clearDisplay();
              nscycle = settsmode - 6;
              if (WiFi.encryptionType(nscycle) == WIFI_AUTH_OPEN) {
              } else {
                settsmode = 11;
                passcur = 0;
                num = 0;
                Mtext = "     ";
                if (WiFi.SSID(nscycle) != ssidIII) ClearPassword();
                else passcur = 29;
              }
              Bcheck = 1;
            } else {
              display.setTextSize(1);
              display.setCursor(10, 0);
              display.print("Available Networks");
              netcycle = 0;
              nety = 12;
              while (netcycle <= netcount) {
                display.setCursor(2, nety);
                display.print(WiFi.SSID(netcycle));
                display.print((WiFi.encryptionType(netcycle) == WIFI_AUTH_OPEN) ? "" : "*");
                netcycle++;
                nety = nety + 10;
              }
              if (settsmode == 6) {
                display.fillRect(0, 10, 127, 10, SSD1306_INVERSE);
              }
              if (settsmode == 7) {
                display.fillRect(0, 20, 127, 10, SSD1306_INVERSE);
              }
              if (settsmode == 8) {
                display.fillRect(0, 30, 127, 10, SSD1306_INVERSE);
              }
              if (settsmode == 9) {
                display.fillRect(0, 40, 127, 10, SSD1306_INVERSE);
              }
              if (settsmode == 10) {
                display.fillRect(0, 50, 127, 10, SSD1306_INVERSE);
              }
            }
          }
        }
        if (settsmode == 11) {
          if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
            settsmode = nscycle + 6;
            Bcheck = 1;
          }
          passwordIII[passcur] = '_';
          display.setTextSize(1);
          display.setCursor(20, 20);
          display.print("Enter Password");
          display.setCursor(0, 0);
          display.print(WiFi.SSID(nscycle));
          display.setTextSize(2);
          display.setCursor(37, 50);
          display.print(Mtext);
          if (passcur > 8) display.setTextSize(1);
          display.setCursor(3, 33);
          display.print(passwordIII);
          display.fillRoundRect(0, 30, 127, 20, 5, SSD1306_INVERSE);
          if ((digitalRead(RD) == 0) && (Bcheck == 0)) {
            Mtext[num] = '-';
            num++;
            Bcheck = 1;
          }
          if ((digitalRead(MD) == 0) && (Bcheck == 0)) {
            Mtext[num] = '.';
            num++;
            Bcheck = 1;
          }
          if ((digitalRead(RU) == 0) && (Bcheck == 0)) {
            Mtext[num] = ' ';
            if (passcur < 30) {
              MorseTranslate();
              passwordIII[passcur] = MorseChar[1];
              if ((Mtext[4] == ' ') && (passcur < 31)) passcur++;
              passwordIII[passcur] = '_';
              num = 0;
              Mtext = "     ";
            }
            Bcheck = 1;
          }
          if ((digitalRead(MU) == 0) && (Bcheck == 0)) {
            passwordIII[passcur] = '\0';
            passcur = passcur - 1;
            if (passcur < 0) passcur = 0;
            Bcheck = 1;
          }
          if ((digitalRead(LD) == 0) && (Bcheck == 0)) {
            passwordIII[passcur] = '\0';
            int ssid_len = WiFi.SSID(nscycle).length() + 1;
            WiFi.SSID(nscycle).toCharArray(ssidIII, ssid_len);
            EEPROM.put(242, ssidIII);
            EEPROM.put(275, passwordIII);
            EEPROM.commit();
            display.clearDisplay();
            display.setCursor(0, 15);
            display.print("Joining   Network...");
            display.display();
            WiFi.begin(ssidIII, passwordIII);
            while (Bcheck == 0) {
              if (WiFi.status() == WL_CONNECTED) {
                Mode = 0;
                Wf = 1;
                Getime = 2;
                Knownet = 1;
                wifitry = 0;
                Bcheck = 1;
              }
              if (digitalRead(LU) == 0) {
                Bcheck = 1;
                settsmode = nscycle + 6;
              }
              delay(500);
            }
            Bcheck = 1;
          }
        }
      }
      if (Mode == 2) {
        if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
          Mode = 0;
          Bcheck = 1;
        }
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("Temp:");
        display.print(Temp);
        display.print(" F");
        display.setTextSize(1);
        display.setCursor(85, 0);
        display.print("o");
        display.setTextSize(2);
        display.setCursor(0, 20);
        display.print("Humid:");
        display.print(Humidity);
        display.print("%");
        display.setCursor(0, 40);
        display.print("Wind:");
        display.print(Wind);
        display.print("mph");
      }
      if (Mode == 3) {
        Message.text[cur] = '_';
        display.setTextSize(1);
        display.setCursor(40, 0);
        display.print("Messages");
        display.setCursor(15, 35);
        display.print(Message.text);
        display.setTextSize(2);
        display.setCursor(37, 15);
        if (SendStatus == 2) display.print(Mtext);
        else {
          display.setCursor(20, 13);
          if (SendStatus == 0) {
            display.print("Sent");
            Message.text[cur] = ' ';
            RMessageVI = RMessageV;
            RMessageV = RMessageIV;
            RMessageIV = RMessageIII;
            RMessageIII = RMessageII;
            RMessageII = RMessage;
            RMessage = Message;
            ClearMessage();
            EEPROM.put(2, RMessage);
            EEPROM.put(42, RMessageII);
            EEPROM.put(82, RMessageIII);
            EEPROM.put(122, RMessageIV);
            EEPROM.put(162, RMessageV);
            EEPROM.put(202, RMessageVI);
            EEPROM.commit();
          }
          if (SendStatus == 1) display.print("Not Sent");
          display.display();
          delay(1000);
          SendStatus = 2;
        }
        if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
          Mode = 0;
          Bcheck = 1;
        }
        if ((digitalRead(LD) == 0) && (Bcheck == 0)) {  //SEND
          Message.text[cur] = ' ';
          esp_now_send(broadcastAddress, (uint8_t*)&Message, sizeof(Message));
          Bcheck = 1;
        }
        if ((digitalRead(RD) == 0) && (Bcheck == 0)) {
          Mtext[num] = '-';
          num++;
          Bcheck = 1;
        }
        if ((digitalRead(MD) == 0) && (Bcheck == 0)) {
          Mtext[num] = '.';
          num++;
          Bcheck = 1;
        }
        if ((digitalRead(RU) == 0) && (Bcheck == 0)) {
          Mtext[num] = ' ';
          if (cur < 30) {
            MorseTranslate();
            Message.text[cur] = MorseChar[1];
            if ((Mtext[4] == ' ') && (cur < 31)) cur++;
            Message.text[cur] = '_';
            num = 0;
            Mtext = "     ";
          }
          Bcheck = 1;
        }
        if ((digitalRead(MU) == 0) && (Bcheck == 0)) {
          Message.text[cur] = ' ';
          cur = cur - 1;
          if (cur < 0) cur = 0;
          Bcheck = 1;
        }
      }
      if (Mode == 4) {  //Received Text
        Unread = 0;
        display.setTextSize(1);
        display.setCursor(40, 0);
        display.print("Messages");
        display.setCursor(0, 10);
        if (Scroll == 0) {
          display.print(RMessageIII.id[0]);
          display.print(": ");
          display.print(RMessageIII.text);
        }
        if (Scroll == 1) {
          display.print(RMessageIV.id[0]);
          display.print(": ");
          display.print(RMessageIV.text);
        }
        if (Scroll == 2) {
          display.print(RMessageV.id[0]);
          display.print(": ");
          display.print(RMessageV.text);
        }
        if (Scroll == 3) {
          display.print(RMessageVI.id[0]);
          display.print(": ");
          display.print(RMessageVI.text);
        }
        display.setCursor(0, 28);
        if (Scroll == 0) {
          display.print(RMessageII.id[0]);
          display.print(": ");
          display.print(RMessageII.text);
        }
        if (Scroll == 1) {
          display.print(RMessageIII.id[0]);
          display.print(": ");
          display.print(RMessageIII.text);
        }
        if (Scroll == 2) {
          display.print(RMessageIV.id[0]);
          display.print(": ");
          display.print(RMessageIV.text);
        }
        if (Scroll == 3) {
          display.print(RMessageV.id[0]);
          display.print(": ");
          display.print(RMessageV.text);
        }
        display.setCursor(0, 48);
        if (Scroll == 0) {
          display.print(RMessage.id[0]);
          display.print(": ");
          display.print(RMessage.text);
        }
        if (Scroll == 1) {
          display.print(RMessageII.id[0]);
          display.print(": ");
          display.print(RMessageII.text);
        }
        if (Scroll == 2) {
          display.print(RMessageIII.id[0]);
          display.print(": ");
          display.print(RMessageIII.text);
        }
        if (Scroll == 3) {
          display.print(RMessageIV.id[0]);
          display.print(": ");
          display.print(RMessageIV.text);
        }
        if ((digitalRead(LU) == 0) && (Bcheck == 0)) {
          Mode = 0;
          Bcheck = 1;
        }
        if ((digitalRead(LD) == 0) && (Bcheck == 0)) {
          Mode = 3;
          Bcheck = 1;
        }
        if (digitalRead(RU) == 0) {
          Scroll++;
          delay(100);
        }
        if (digitalRead(RD) == 0) {
          Scroll--;
          delay(100);
        }
        if (Scroll > 3) Scroll = 3;
        if (Scroll < 0) Scroll = 0;
      }
    }
  }
  display.display();
  delay(1);
  if ((Display == 1) && (StartMess != 1)) display.clearDisplay();
}
void BatteryLevel() {
  result = analogRead(A13);
  if (result > 20) {
    Charging = 1;
    if (prevresult < 20) {
      Display = 1;
      Time = microseconds;
    }
  }
  if (result < 20) {
    Charging = 0;
    if (prevresult > 20) {
      Display = 1;
      Time = microseconds;
    }
  }
  if (Charging == 0) {
    if (result > 15) Battery = 16;
    if (result < 15) Battery = 15;
    if (result < 14) Battery = 14;
    if (result < 13) Battery = 13;
    if (result < 12) Battery = 12;
    if (result < 11) Battery = 11;
    if (result < 10) Battery = 9;
    if (result < 9) Battery = 8;
    if (result < 8) Battery = 7;
    if (result < 7) Battery = 6;
    if (result < 6) Battery = 5;
    if (result < 5) Battery = 4;
    if (result < 4) Battery = 3;
    if (result < 3) Battery = 2;
    if (result < 2) Battery = 0;
  }
  prevresult = result;
}
void ClearMessage() {
  cur = 0;
  while (cur < 31) {
    Message.text[cur] = ' ';
    cur++;
  }
  cur = 0;
}
void ClearPassword() {
  passcur = 0;
  while (passcur < 31) {
    passwordIII[passcur] = '\0';
    passcur++;
  }
  passcur = 0;
}
void EspNowPrep() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0) payload = http.getString();
  http.end();
  return payload;
}
void MorseTranslate() {
  if ((Mtext[0] == '.') && (Mtext[1] == '-')) {
    if (Mtext[2] == ' ') {
      MorseChar[1] = 'A';
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'B';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '.')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'C';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'D';
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == ' ')) {
    MorseChar[1] = 'E';
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '.')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'F';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '.') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'G';
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'H';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if (Mtext[2] == ' ') {
      MorseChar[1] = 'I';
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '-')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'J';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '-') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'K';
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'L';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if (Mtext[2] == ' ') {
      MorseChar[1] = 'M';
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if (Mtext[2] == ' ') {
      MorseChar[1] = 'N';
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'O';
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '.')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'P';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '-')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'Q';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '.') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'R';
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'S';
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == ' ')) {
    MorseChar[1] = 'T';
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '-') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'U';
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '-')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'V';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == ' ')) {
      MorseChar[1] = 'W';
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '-')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'X';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '-')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'Y';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == ' ') {
        MorseChar[1] = 'Z';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '-')) {
      if (Mtext[4] == '-') {
        MorseChar[1] = '1';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '-')) {
      if (Mtext[4] == '-') {
        MorseChar[1] = '2';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '-')) {
      if (Mtext[4] == '-') {
        MorseChar[1] = '3';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == '-') {
        MorseChar[1] = '4';
      }
    }
  }
  if ((Mtext[0] == '.') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == '.') {
        MorseChar[1] = '5';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '.')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == '.') {
        MorseChar[1] = '6';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '.') && (Mtext[3] == '.')) {
      if (Mtext[4] == '.') {
        MorseChar[1] = '7';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '.')) {
      if (Mtext[4] == '.') {
        MorseChar[1] = '8';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '-')) {
      if (Mtext[4] == '.') {
        MorseChar[1] = '9';
      }
    }
  }
  if ((Mtext[0] == '-') && (Mtext[1] == '-')) {
    if ((Mtext[2] == '-') && (Mtext[3] == '-')) {
      if (Mtext[4] == '-') {
        MorseChar[1] = '0';
      }
    }
  }
  if (Mtext[0] == ' ') {
    MorseChar[1] = ' ';
  }
}
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  //if status==0 then successfully sent
  SendStatus = status;
}
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  RMessageVI = RMessageV;
  RMessageV = RMessageIV;
  RMessageIV = RMessageIII;
  RMessageIII = RMessageII;
  RMessageII = RMessage;
  memcpy(&RMessage, incomingData, sizeof(RMessage));
  EEPROM.put(2, RMessage);
  EEPROM.put(42, RMessageII);
  EEPROM.put(82, RMessageIII);
  EEPROM.put(122, RMessageIV);
  EEPROM.put(162, RMessageV);
  EEPROM.put(202, RMessageVI);
  EEPROM.commit();
  NewMessage = 1;
  Unread++;
  Display = 1;
}
void Restart() {
  while (digitalRead(MU) == 0) {
    display.clearDisplay();
    display.fillRect(0, 0, 130, StartMess - 5, SSD1306_WHITE);
    display.display();
    StartMess--;
    if (StartMess < 2) ESP.restart();
    delay(6);
  }
}
void TimeKeeping() {
  if (Getime == 1) {
    display.setTextSize(2);
    netcycle = 0;
    netcount = WiFi.scanNetworks();
    if (netcount == 0) {
      display.clearDisplay();
      display.print("No Networks");
      StartMess = 1;
    }
    while (netcycle <= netcount) {
      if (WiFi.SSID(netcycle) == ssid) {
        Knownet = 1;
        StartMess = 1;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print(ssid);
        display.print(" Found");
        WiFi.begin(ssid, password);
        netcycle = netcount;
      }
      if (WiFi.SSID(netcycle) == ssidII) {
        Knownet = 1;
        StartMess = 1;
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(ssidII);
        display.print(" Found");
        WiFi.begin(ssidII, passwordII);
        netcycle = netcount;
      }
      if (WiFi.SSID(netcycle) == ssidIII) {
        Knownet = 1;
        StartMess = 1;
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(ssidIII);
        display.print(" Found");
        WiFi.begin(ssidIII, passwordIII);
        netcycle = netcount;
      }
      netcycle++;
    }
    display.display();
    Getime = 2;
  }
  if (Getime == 2) {
    if (Connected == 1) {
      configTime(-18000, 3600, "pool.ntp.org");
      getLocalTime(&timeinfo);
      Getime = 0;
      Getweather = 1;
      prevmicros = microseconds;
      wifitry = 0;
      StartMess = 0;
    }
    if (Knownet == 0) {
      StartMess = 2;
      Getime = 3;
      EspNowPrep();
      display.clearDisplay();
    }
    while ((WiFi.status() != WL_CONNECTED) && (wifitryII <= 2)) {
      wifitry++;
      delay(500);
      if (wifitry > 10) {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(true);
        wifitry = 0;
        wifitryII++;
        if (wifitryII > 2) {
          StartMess = 2;
          Getime = 3;
          EspNowPrep();
          display.clearDisplay();
        }
      }
    }
  }
  if (Getime == 0) getLocalTime(&timeinfo);
  strftime(Seconds, 3, "%S", &timeinfo);
  strftime(Minutes, 3, "%M", &timeinfo);
  strftime(Hours, 3, "%I", &timeinfo);
  strftime(Days, 3, "%e", &timeinfo);
  strftime(Months, 4, "%b", &timeinfo);
  microseconds = millis();
  if ((microseconds - Time > Screenoff * 1000) && (Display == 1) && (Screenoff != 61)) {
    Display = 0;
    display.clearDisplay();
  }
  if (microseconds - hourmicros > 3600000) {
    Getime = 1;
    hourmicros = microseconds;
  }
}
void WiFiSymbol() {
  display.drawPixel(15, 11, SSD1306_WHITE);
  display.drawPixel(12, 9, SSD1306_WHITE);
  display.drawPixel(13, 8, SSD1306_WHITE);
  display.drawPixel(14, 7, SSD1306_WHITE);
  display.drawPixel(15, 7, SSD1306_WHITE);
  display.drawPixel(16, 7, SSD1306_WHITE);
  display.drawPixel(17, 8, SSD1306_WHITE);
  display.drawPixel(18, 9, SSD1306_WHITE);
  display.drawPixel(10, 7, SSD1306_WHITE);
  display.drawPixel(11, 6, SSD1306_WHITE);
  display.drawPixel(12, 5, SSD1306_WHITE);
  display.drawPixel(13, 4, SSD1306_WHITE);
  display.drawPixel(14, 4, SSD1306_WHITE);
  display.drawPixel(15, 4, SSD1306_WHITE);
  display.drawPixel(16, 4, SSD1306_WHITE);
  display.drawPixel(17, 4, SSD1306_WHITE);
  display.drawPixel(18, 5, SSD1306_WHITE);
  display.drawPixel(19, 6, SSD1306_WHITE);
  display.drawPixel(20, 7, SSD1306_WHITE);
  display.drawPixel(8, 5, SSD1306_WHITE);
  display.drawPixel(9, 4, SSD1306_WHITE);
  display.drawPixel(10, 3, SSD1306_WHITE);
  display.drawPixel(11, 2, SSD1306_WHITE);
  display.drawPixel(12, 1, SSD1306_WHITE);
  display.drawPixel(13, 1, SSD1306_WHITE);
  display.drawPixel(14, 1, SSD1306_WHITE);
  display.drawPixel(15, 1, SSD1306_WHITE);
  display.drawPixel(16, 1, SSD1306_WHITE);
  display.drawPixel(17, 1, SSD1306_WHITE);
  display.drawPixel(18, 1, SSD1306_WHITE);
  display.drawPixel(19, 2, SSD1306_WHITE);
  display.drawPixel(20, 3, SSD1306_WHITE);
  display.drawPixel(21, 4, SSD1306_WHITE);
  display.drawPixel(22, 5, SSD1306_WHITE);
}
