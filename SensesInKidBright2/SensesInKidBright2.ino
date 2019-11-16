//Senses Platform in KidBright (ESP32)
//Part 2 : Send B1 B2 click to server

#include "Senses_wifi_esp32.h"
#include <Wire.h> 
#include <Adafruit_GFX.h> 
#include "Adafruit_LEDBackpack.h" 

#define B1_PIN 16
#define B2_PIN 14

#define BTN_SLOT 2
#define TIME_SLOT 1

#define LCD_DELAY 250

#define B1_SLOT 1
#define B2_SLOT 2
#define SENSES_DELAY 10000

int slotth;
unsigned long pms, ms, dms;
unsigned long timer[TIME_SLOT];

int btnth;
boolean btn[BTN_SLOT], btnPrev[BTN_SLOT];
unsigned long btnClick[BTN_SLOT] = {0, 0};

boolean ripple[BTN_SLOT][3] = {{false, false, false}, {false, false, false}};

//Change these setting before upload to board
const char *ssid = "your-wifi-network-name";
const char *passw = "your-wifi-password";
const char *userid = "your-senses-user-id";
const char *key = "your-device-key";

String response;

Senses_wifi_esp32 myiot;
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix(); 

static const uint8_t PROGMEM ripple_ani[][8] = {
  { B00000000, B00000000, B00000000, B00011000, B00011000, B00000000, B00000000, B00000000 }, //0
  { B00000000, B00000000, B00111100, B00100100, B00100100, B00111100, B00000000, B00000000 }, //1
  { B00000000, B01111110, B01000010, B01011010, B01011010, B01000010, B01111110, B00000000 }, //2
  { B00000000, B01111110, B01000010, B01011010, B01011010, B01000010, B01111110, B00000000 }, //3
  { B11111111, B10000001, B10000001, B10011001, B10011001, B10000001, B10000001, B11111111 }, //4
  { B11111111, B10000001, B10111101, B10100101, B10100101, B10111101, B10000001, B11111111 }, //5
  { B11111111, B10000001, B10000001, B10011001, B10011001, B10000001, B10000001, B11111111 }, //6
  { B11111111, B10000001, B10111101, B10100101, B10100101, B10111101, B10000001, B11111111 }, //7
};
    
//Tick timers
void tickTimers() {
  pms = ms; ms = millis();
  if(pms < ms) { dms = ms - pms; }
  else { dms = ULONG_MAX - pms + ms; }
  for(slotth = 0; slotth < TIME_SLOT; slotth++) { timer[slotth] += dms; }
}

//Tick button
void tickButtons() {
  for(btnth = 0; btnth < BTN_SLOT; btnth++) {
    btnPrev[btnth] = btn[btnth];
  }
  
  btn[0] = digitalRead(B1_PIN);
  btn[1] = digitalRead(B2_PIN);

  for(btnth = 0; btnth < BTN_SLOT; btnth++) {
    if(btnPrev[btnth] && !btn[btnth]) {
      Serial.print(F("Bounce "));
      Serial.println(btnth);
      btnClick[btnth]++;
      ripple[btnth][0] = true;
    }
  }
}

//Send data to Senses Platform periodically
void taskSensesSend(void * pvParameters) {
  response = myiot.connect(ssid, passw, userid, key);
  while(true) {
    myiot.send(B1_SLOT, btnClick[0]);
    myiot.send(B2_SLOT, btnClick[1]);
    Serial.print(F("B1 Click : "));
    Serial.println(btnClick[0]);
    Serial.print(F("B2 Click : "));
    Serial.println(btnClick[1]);
    Serial.println(F("Send data to Senses Platform"));
    delay(SENSES_DELAY);
  }
}

void setup(){
  pinMode(B1_PIN, INPUT_PULLUP);
  pinMode(B2_PIN, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.println(F("Senses Platform in KidBright"));
  
  xTaskCreatePinnedToCore(taskSensesSend, "taskSensesSend", 4096, NULL, 3, NULL, 0);
  
  matrix.begin(0x70);
  matrix.clear();
  matrix.writeDisplay();
}

int r1, r2;

void loop( ){
  tickTimers();
  tickButtons();

  //Matrix display
  if(timer[0] >= LCD_DELAY) {
    matrix.clear();
    r1 = 4 * ripple[0][2] + 2 * ripple[0][1] + ripple[0][0];
    r2 = 4 * ripple[1][2] + 2 * ripple[1][1] + ripple[1][0];
    matrix.drawBitmap(0, 8, ripple_ani[r1], 8, 8, LED_ON);
    matrix.drawBitmap(0, 0, ripple_ani[r2], 8, 8, LED_ON);
    matrix.writeDisplay();

    //Ripple
    ripple[0][2] = ripple[0][1]; ripple[0][1] = ripple[0][0]; ripple[0][0] = false;
    ripple[1][2] = ripple[1][1]; ripple[1][1] = ripple[1][0]; ripple[1][0] = false;
    timer[0] = 0;
  }

  delay(1);
}
