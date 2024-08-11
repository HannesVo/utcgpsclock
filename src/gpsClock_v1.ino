#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Board
#define SERIAL_BAUD 9600

// OnBoard LED PIN
#define LED_1_PIN 22

// GPS 
#define GPS_PIN_TX 25 //33
#define GPS_PIN_RX 33 //32
#define GPS_BAUD 9600

// Lightsensor
#define LIGHT_SENSOR_PIN 13
//    Sensor  max brightness is 4095
#define SENSOR_MAX_BRIGHTNESS 4095
int SnsMaxBrtn = 4095; 

//MATRIX
//define Matrix
#define MTX_HARDWARE_TYPE MD_MAX72XX::FC16_HW
//PINS
#define MTX_MAX_DEVICES 4
#define MTX_CS_PIN 27
#define MTX_DATA_PIN 26  //DIN
#define MTX_CLK_PIN 14
//Display max brightness is 15
#define DISPLAY_MAX_BRIGHTNESS 15
int dplMaxBrtn = 15;

int noTimeCount = 0;

// ### Declare global Objects ###
// GPS
EspSoftwareSerial::UART myGpsPort;
TinyGPSPlus myGps;

bool validTime = false;

//DISPLAY
MD_Parola myDisplay = MD_Parola(MTX_HARDWARE_TYPE, MTX_DATA_PIN, MTX_CLK_PIN, MTX_CS_PIN, MTX_MAX_DEVICES);

float factorBrightness = float(SENSOR_MAX_BRIGHTNESS) / float(DISPLAY_MAX_BRIGHTNESS);
char TIME_BUF[6];
  
/// ############ SETUP############ //
void setup() {

  //LightUp Onboard LED to show board is working
  pinMode(LED_1_PIN,OUTPUT);
  digitalWrite(LED_1_PIN,LOW);

  Serial.begin(SERIAL_BAUD); // Standard hardware serial port

  myGpsPort.begin(GPS_BAUD, SWSERIAL_8N1, GPS_PIN_RX, GPS_PIN_TX, false);
  if (!myGpsPort) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
    while (1) { // Don't continue with invalid configuration
      delay (1000);
    }
  }
  
  //prePareDisplay
  myDisplay.begin();
  myDisplay.setIntensity(DISPLAY_MAX_BRIGHTNESS);
  myDisplay.setInvert(true);
  myDisplay.displayClear();
  delay(500);
  myDisplay.setInvert(false);
  myDisplay.setTextAlignment(PA_CENTER);
  //myDisplay.print("no time");
  myDisplay.displayClear();
}

void loop() {
  int cnt = 0;
  String strtime = "";
  while(myGpsPort.available() > 0){
    //DO SOME GPS
    if (myGps.encode(myGpsPort.read())){
      if (myGps.time.isValid() && myGps.time.hour() != 0){
        if (myGps.time.hour() < 10) strtime = String(strtime + F("0"));
        strtime = String(strtime + myGps.time.hour() + F(":"));       
        if (myGps.time.minute() < 10) strtime = String(strtime + F("0"));
        strtime = String(strtime + myGps.time.minute());
        Serial.println(strtime);
        strtime.toCharArray(TIME_BUF,6);
        validTime = true;
      }else{
        validTime = false;
        Serial.println("no GPS-Time available");
      }
    }
    
  }
  // DO SOMETHING OTHER
  // prepare displaysettings
  int sensorValue = analogRead(LIGHT_SENSOR_PIN);
  int displayBrightness = int((sensorValue) / factorBrightness);
  if(displayBrightness <= 1){displayBrightness = int(1);}
  myDisplay.setIntensity(displayBrightness);
  //Serial.println("Sval: " + String(sensorValue) + " - Factor: " + String(factorBrightness) + " - DisplayBrightness: " + String(displayBrightness));

  //show something on the display
  if(validTime){
    myDisplay.setTextAlignment(PA_CENTER);
    myDisplay.setIntensity(displayBrightness);
    //myDisplay.print(TIME_BUF);
    myDisplay.displayText(TIME_BUF,PA_CENTER,50,100,PA_NO_EFFECT,PA_NO_EFFECT);
     if (myDisplay.displayAnimate()) {    // animate the display
      myDisplay.displayReset();          // reset the current animation
    }
  }else{
    noTimeCount += 1;
    Serial.println("show waiting message");
    myDisplay.displayReset();
    myDisplay.displayClear();
    myDisplay.displayText(" no GPS-Time available                    ", PA_LEFT, 25,3000,PA_SCROLL_LEFT, PA_NO_EFFECT);
    while (cnt <= 5000){
      if (myDisplay.displayAnimate()) {    // animate the display
        myDisplay.displayReset();          // reset the current animation
      }
      delay(1);
      cnt++;
    }
    cnt = 0;
  }


}
