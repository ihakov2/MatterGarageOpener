/*
MIT License
Matter garage opener
Copyright (c) 2024 ILDUS HAKOV ihakov@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


Open/Close/Status garage over the internet using mobile phone. Supports voice commands.

Compatible with old style garage door openers that use a single-button remote control.

      Requires:
        - Compatible Matter board.
        - the hat or board with relay and ultrasonic sensor SR04.
        - Matter Hub.

      Based on SiliconLabs matter_lightbulb example.

    The device has to be commissioned to a Matter hub first.

    Built on Sparkfun SparkFun Thing Plus Matter - MGM240P with SiliconLabs libraries to use with Google Nest Hub 2nd generation
    but should work with any matter hub - not tested.

   Author: ildus hakov 5/26/2024 

  This code for the board v1.2 (Ver2 board) 
 */
#include <Matter.h>
#include <MatterLightbulb.h>
// Uncomment below line to see the debug output 
//#define SHOW_SERIAL
#define TRIGGER PD2
#define ECHO    PD3
#define TRANS   PD0
#define DIST_DETECTION  70 //cm
#define CONTACT_CLOSE_DURATION 700 //msec
float dist_inches, dist_cm;

MatterLightbulb matter_bulb_1;

bool prevRemoteStatus = false, remoteStatus = false;
bool prevLocalStatus = false, localStatus = false;
bool garageOpenClosedEvent = false;
unsigned long previousMillis = 0;
const long garageOpenCloseDuration = 7000; // 7secs

void setup() {
  Serial.begin(115200);
  Matter.begin();
  matter_bulb_1.begin();
  Serial.println("Starting device");
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(TRIGGER,OUTPUT);
	pinMode(ECHO,INPUT);
  pinMode(TRANS, OUTPUT);
  digitalWrite(TRANS, LOW);
  
  if (!Matter.isDeviceCommissioned()) {
      Serial.println("Matter device is not commissioned");
      Serial.println("Commission it to your Matter hub with the manual pairing code or QR code");
      Serial.printf("Manual pairing code: %s\n", Matter.getManualPairingCode().c_str());
      Serial.printf("QR code URL: %s\n", Matter.getOnboardingQRCodeUrl().c_str());
  }
  while (!Matter.isDeviceCommissioned()) {
    delay(200);
  }

  if (!Matter.isDeviceConnected()) {
      Serial.println("Waiting for network connection...");
  }
  while (!Matter.isDeviceConnected()) {
    delay(200);
  }
  Serial.println("Garage opener device connected");
}

void loop() {
  calcDistance();
  // local status
  bool localStatusChanged = false;  
  localStatus = dist_cm < DIST_DETECTION;
  
  unsigned long currentMillis = millis();
  if(garageOpenClosedEvent) {
  // Wait 7-8 seconds
    if ((previousMillis + garageOpenCloseDuration) < millis()) {
      prevLocalStatus=!prevLocalStatus;
      garageOpenClosedEvent=false;
      //printLocal();
    }
  }
  if(localStatus && !prevLocalStatus) {    
    matter_bulb_1.set_onoff(true); // set garage opened status
    localStatusChanged = true;
    #ifdef SHOW_SERIAL
      printLocal();
      Serial.println("Status: Garage is opened");
      printDist();
    #endif

    digitalWrite(LED_BUILTIN, HIGH);
    prevLocalStatus = localStatus;
  }
  if(!localStatus && prevLocalStatus) {
    matter_bulb_1.set_onoff(false); // set garage closed status  
    localStatusChanged = true;
    #ifdef SHOW_SERIAL
      printLocal();
      Serial.println("Status: Garage is closed");
      printDist();
    #endif    
    digitalWrite(LED_BUILTIN, LOW);
    prevLocalStatus = localStatus;
  }
  /// Read remote
  remoteStatus = matter_bulb_1.get_onoff();
 
  //open
  if(remoteStatus != prevRemoteStatus) { 
    #ifdef SHOW_SERIAL
      printRemote();
    #endif
    if(!localStatusChanged) {
      openCloseGarage();
    }else {
      localStatusChanged=false;   // ignore opening garage, it is already opened by someone.
    }
    prevRemoteStatus = remoteStatus;    
  }
  //close
  if(prevRemoteStatus != remoteStatus) { 
    #ifdef SHOW_SERIAL
      printRemote();
    #endif
    if(!localStatusChanged) {
      openCloseGarage();
    }else {
      localStatusChanged=false;   // ignore opening garage, it is already opened by someone.
    }
    prevRemoteStatus = remoteStatus;
  }
  delay(1000);
}

  void openCloseGarage() {
    digitalWrite(TRANS, HIGH);
    delay(CONTACT_CLOSE_DURATION);
    digitalWrite(TRANS, LOW);
    garageOpenClosedEvent=true;
    previousMillis = millis();
    #ifdef SHOW_SERIAL
      Serial.println("openCloseGarage");
    #endif
  }

  void calcDistance() {
    digitalWrite(TRIGGER,LOW);
		delayMicroseconds(5);
		
		//Start Measurement
		digitalWrite(TRIGGER,HIGH);
		delayMicroseconds(10);
		digitalWrite(TRIGGER,LOW);
		
     //If pulseIn not working try to change it to HIGH
		float distance=pulseIn(ECHO,LOW) * 0.0001657;
		dist_inches=distance*39.37;
    dist_cm=dist_inches*2.54;    
  }

  void printDist() {
    Serial.print(dist_cm);Serial.println(" cm");
  }
 
void printLocal() {  
  Serial.print("localStatus=");Serial.print(localStatus);
  Serial.print(" prevLocalStatus=");Serial.println(prevLocalStatus);
}

void printRemote() {   
    Serial.print("remoteStatus=");Serial.print(remoteStatus);
    Serial.print(" prevRemoteStatus=");Serial.println(prevRemoteStatus);
}
