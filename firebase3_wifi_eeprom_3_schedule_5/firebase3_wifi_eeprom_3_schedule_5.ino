// wifi control
#include <WiFi.h>
#include "esp_wifi.h"
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String date0, date1, date2, date3, date4, uv;
unsigned long start_time0, start_time1, start_time2, start_time3, start_time4;
unsigned long end_time0, end_time1, end_time2, end_time3, end_time4;

const uint16_t port1 = 5555;
const char * host;
WiFiClient client;

#define FIREBASE_HOST "https://sterilizer.firebaseio.com/"
#define FIREBASE_AUTH "pnUwQVHcBf5AJ0fObO7d5oVDmnYIS87E1VQgiNQT"

//Define FirebaseESP32 data object
FirebaseData firebaseData;
FirebaseJson json;

//EEPROM header
#include <EEPROM.h>

// timers
unsigned long timer1;

const int uv_light = 5;
const int indication = 26;
const int ACin = 27;
const int sensor1 = 18;
const int sensor1_power = 25;
const int led = 2;
const int motion_signal = 15;
const int motion_signal_user = 4;

String line;
byte l, j, o, len;

boolean flag = false, flag2 = false, flag3 = false;
unsigned long checker = 0, checker2, counter1 = 0, checker3, current_seconds;
int currentDay, currentHour, currentMin, currentSec, value, motion_value;

void setup() {

  Serial.begin(115200);
  
  pinMode(motion_signal, OUTPUT);
  digitalWrite(motion_signal, HIGH);

  pinMode(motion_signal_user, OUTPUT);
  digitalWrite(motion_signal_user, HIGH);

  pinMode(led, OUTPUT);
  digitalWrite(led,  LOW);

  pinMode(uv_light, OUTPUT);
  digitalWrite(uv_light, HIGH);

  pinMode(indication, OUTPUT);
  digitalWrite(indication, HIGH);
  
  pinMode(ACin, OUTPUT);
  digitalWrite(ACin, HIGH);

  pinMode(sensor1, INPUT_PULLUP);

  pinMode(sensor1_power, OUTPUT);
  digitalWrite(sensor1_power, HIGH);

  EEPROM.begin(500); //Initialasing EEPROM
  delay(10);

  Serial.println("Reading EEPROM ssid");

      String esid;
      for (int i = 0; i < 32; ++i)
      {
        esid += char(EEPROM.read(i));
      }
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(esid);
      Serial.println(esid.c_str());
      Serial.println("Reading EEPROM pass");
   
      String epass = "";
      for (int i = 32; i < 96; ++i)
      {
        epass += char(EEPROM.read(i));
      }
      Serial.print("PASS: ");
      Serial.println(epass);
      Serial.println(epass.c_str());

      WiFi.begin(esid.c_str(), epass.c_str());
     
      while ((WiFi.status() != WL_CONNECTED))
      {
       delay(1);        
       if(checker < 30000){
        checker++;
       }
       if(checker > 20000){
        WiFi.softAP("razecov", "razecov123");
        digitalWrite(led,  HIGH);
        while(1){
       
         while(!(client.connected())){
          connection();
         }
         while((client.connected())){
          Serial.println("Connected to TCP server 1");
          Credential_listener();  // end of this function eeprom locations is filled with 1 and credentials then return.
         }

        }
           
       }
       
      }

      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      Firebase.reconnectWiFi(true);
 
      //Set database read timeout to 1 minute (max 15 minutes)
      Firebase.setReadTimeout(firebaseData, 1000 * 60);
      //tiny, small, medium, large and unlimited.
      //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
      Firebase.setwriteSizeLimit(firebaseData, "tiny");
     
      Serial.println("------------------------------------");
      Serial.println("Connected to Firebase...");

      delay(5000);
      digitalWrite(ACin, LOW);
      digitalWrite(indication, LOW);

      Serial.println();
      Serial.print("IP_Address: ");
      Serial.println(WiFi.localIP());

 // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(19800);

  purifier();

}

void purifier(){
  
  while(1){
    
    if(Firebase.get(firebaseData, "/IP210505/uv")){

      uv = firebaseData.stringData();

      if(uv == "ON"){
       digitalWrite(uv_light, LOW);
       digitalWrite(sensor1_power, LOW);
       checker3++; 
      }
      else if(uv == "OFF"){
       digitalWrite(sensor1_power, HIGH); 
       digitalWrite(uv_light, HIGH);
       digitalWrite(motion_signal, HIGH);
       checker3 = 0;
      }
      
    }

    Serial.println(checker3);
    if(value == 0){
     if(checker3 >= 30){
      digitalWrite(motion_signal, LOW);  
     }
    }
    else if(value == 1){
     if(checker3 >= 12){
      digitalWrite(motion_signal, LOW);  
     }  
    }
    else if((value == 2) || ((value == 3))){
     if(checker3 >= 7){
      digitalWrite(motion_signal, LOW);  
     }  
    }
    else if((value == 4) || ((value == 5))){
     if(checker3 >= 4){
      digitalWrite(motion_signal, LOW);  
     }  
    }
    

    int val = digitalRead(sensor1);
    if(val == 0){
     Firebase.setInt(firebaseData, "/IP210505/motionDetected", 2);
     digitalWrite(sensor1_power, HIGH); 
     digitalWrite(uv_light, HIGH);
     checker3 = 0;
     digitalWrite(motion_signal, HIGH);
     Firebase.setString(firebaseData, "/IP210505/uv", "OFF");
     while(1){
      if(Firebase.get(firebaseData, "/IP210505/motionDetected")){
       motion_value = firebaseData.intData();
       if(motion_value == 1){
        digitalWrite(motion_signal_user, LOW);
        delay(2000);
        digitalWrite(motion_signal_user, HIGH);
        digitalWrite(motion_signal, HIGH);
        break;
       }

      } 
     }
    }
    
    if(Firebase.get(firebaseData, "/IP210505/schedules/active")){

     value = firebaseData.intData();

    }

     if(value == 1){

     // schedules 1 //
     
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/dayStamp")){
  
      date0 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/startStamp")){
  
      start_time0 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/endStamp")){
  
      end_time0 = firebaseData.intData();
      
     } 
  
     unsigned long duration0 = (end_time0 - start_time0);
     unsigned long limit0 = duration0 / 2;
     int extender0;
     if(limit0 <= 300){
      extender0 = limit0;
     }
     else{
      extender0 = 300;
     }
     unsigned long start_offset0 = (start_time0 + limit0);
     unsigned long end_offset0 = (end_time0 + extender0);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date0[i] == '1'){

         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
         
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec));
   
         if((start_time0 <= current_seconds) && (current_seconds <= start_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");
         }
         if((end_time0 <= current_seconds) && (current_seconds <= end_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }
                       
       }
      }
     }  
    }

    if(value == 2){

     // schedules 1 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/dayStamp")){
  
      date0 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/startStamp")){
  
      start_time0 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/endStamp")){
  
      end_time0 = firebaseData.intData();
      
     }

     unsigned long duration0 = (end_time0 - start_time0);
     unsigned long limit0 = duration0 / 2;
     int extender0;
     if(limit0 <= 300){
      extender0 = limit0;
     }
     else{
      extender0 = 300;
     }
     unsigned long start_offset0 = (start_time0 + limit0);
     unsigned long end_offset0 = (end_time0 + extender0);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date0[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time0 <= current_seconds) && (current_seconds <= start_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time0 <= current_seconds) && (current_seconds <= end_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }
  
     // schedules 2 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/dayStamp")){
  
      date1 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/startStamp")){
  
      start_time1 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/endStamp")){
  
      end_time1 = firebaseData.intData();
      
     }

     unsigned long duration1 = (end_time1 - start_time1);
     unsigned long limit1 = duration1 / 2;
     int extender1;
     if(limit1 <= 300){
      extender1 = limit1;
     }
     else{
      extender1 = 300;
     }
     unsigned long start_offset1 = (start_time1 + limit1);
     unsigned long end_offset1 = (end_time1 + extender1);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date1[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time1 <= current_seconds) && (current_seconds <= start_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time1 <= current_seconds) && (current_seconds <= end_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }       
    }

    if(value == 3){

     // schedules 1 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/dayStamp")){
  
      date0 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/startStamp")){
  
      start_time0 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/endStamp")){
  
      end_time0 = firebaseData.intData();
      
     }

     unsigned long duration0 = (end_time0 - start_time0);
     unsigned long limit0 = duration0 / 2;
     int extender0;
     if(limit0 <= 300){
      extender0 = limit0;
     }
     else{
      extender0 = 300;
     }
     unsigned long start_offset0 = (start_time0 + limit0);
     unsigned long end_offset0 = (end_time0 + extender0);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date0[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time0 <= current_seconds) && (current_seconds <= start_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time0 <= current_seconds) && (current_seconds <= end_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }
  
     // schedules 2 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/dayStamp")){
  
      date1 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/startStamp")){
  
      start_time1 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/endStamp")){
  
      end_time1 = firebaseData.intData();
      
     }

     unsigned long duration1 = (end_time1 - start_time1);
     unsigned long limit1 = duration1 / 2;
     int extender1;
     if(limit1 <= 300){
      extender1 = limit1;
     }
     else{
      extender1 = 300;
     }
     unsigned long start_offset1 = (start_time1 + limit1);
     unsigned long end_offset1 = (end_time1 + extender1);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date1[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time1 <= current_seconds) && (current_seconds <= start_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time1 <= current_seconds) && (current_seconds <= end_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }

     // schedules 3 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/dayStamp")){
  
      date2 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/startStamp")){
  
      start_time2 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/endStamp")){
  
      end_time2 = firebaseData.intData();
      
     }

     unsigned long duration2 = (end_time2 - start_time2);
     unsigned long limit2 = duration2 / 2;
     int extender2;
     if(limit2 <= 300){
      extender2 = limit2;
     }
     else{
      extender2 = 300;
     }
     unsigned long start_offset2 = (start_time2 + limit2);
     unsigned long end_offset2 = (end_time2 + extender2);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date2[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time2 <= current_seconds) && (current_seconds <= start_offset2)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time2 <= current_seconds) && (current_seconds <= end_offset2)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }
            
    }

    if(value == 4){

     // schedules 1 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/dayStamp")){
  
      date0 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/startStamp")){
  
      start_time0 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/endStamp")){
  
      end_time0 = firebaseData.intData();
      
     }

     unsigned long duration0 = (end_time0 - start_time0);
     unsigned long limit0 = duration0 / 2;
     int extender0;
     if(limit0 <= 300){
      extender0 = limit0;
     }
     else{
      extender0 = 300;
     }
     unsigned long start_offset0 = (start_time0 + limit0);
     unsigned long end_offset0 = (end_time0 + extender0);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date0[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time0 <= current_seconds) && (current_seconds <= start_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time0 <= current_seconds) && (current_seconds <= end_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }
  
     // schedules 2 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/dayStamp")){
  
      date1 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/startStamp")){
  
      start_time1 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/endStamp")){
  
      end_time1 = firebaseData.intData();
      
     }

     unsigned long duration1 = (end_time1 - start_time1);
     unsigned long limit1 = duration1 / 2;
     int extender1;
     if(limit1 <= 300){
      extender1 = limit1;
     }
     else{
      extender1 = 300;
     }
     unsigned long start_offset1 = (start_time1 + limit1);
     unsigned long end_offset1 = (end_time1 + extender1);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date1[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time1 <= current_seconds) && (current_seconds <= start_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time1 <= current_seconds) && (current_seconds <= end_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }

     // schedules 3 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/dayStamp")){
  
      date2 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/startStamp")){
  
      start_time2 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/endStamp")){
  
      end_time2 = firebaseData.intData();
      
     }

     unsigned long duration2 = (end_time2 - start_time2);
     unsigned long limit2 = duration2 / 2;
     int extender2;
     if(limit2 <= 300){
      extender2 = limit2;
     }
     else{
      extender2 = 300;
     }
     unsigned long start_offset2 = (start_time2 + limit2);
     unsigned long end_offset2 = (end_time2 + extender2);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date2[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time2 <= current_seconds) && (current_seconds <= start_offset2)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time2 <= current_seconds) && (current_seconds <= end_offset2)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }

     // schedules 4 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/3/dayStamp")){
  
      date3 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/3/startStamp")){
  
      start_time3 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/3/endStamp")){
  
      end_time3 = firebaseData.intData();
      
     }

     unsigned long duration3 = (end_time3 - start_time3);
     unsigned long limit3 = duration3 / 2;
     int extender3;
     if(limit3 <= 300){
      extender3 = limit3;
     }
     else{
      extender3 = 300;
     }
     unsigned long start_offset3 = (start_time3 + limit3);
     unsigned long end_offset3 = (end_time3 + extender3);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date3[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time3 <= current_seconds) && (current_seconds <= start_offset3)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time3 <= current_seconds) && (current_seconds <= end_offset3)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }
            
    }

    if(value == 5){

     // schedules 1 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/dayStamp")){
  
      date0 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/startStamp")){
  
      start_time0 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/0/endStamp")){
  
      end_time0 = firebaseData.intData();
      
     }

     unsigned long duration0 = (end_time0 - start_time0);
     unsigned long limit0 = duration0 / 2;
     int extender0;
     if(limit0 <= 300){
      extender0 = limit0;
     }
     else{
      extender0 = 300;
     }
     unsigned long start_offset0 = (start_time0 + limit0);
     unsigned long end_offset0 = (end_time0 + extender0);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date0[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time0 <= current_seconds) && (current_seconds <= start_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time0 <= current_seconds) && (current_seconds <= end_offset0)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }
  
     // schedules 2 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/dayStamp")){
  
      date1 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/startStamp")){
  
      start_time1 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/1/endStamp")){
  
      end_time1 = firebaseData.intData();
      
     }

     unsigned long duration1 = (end_time1 - start_time1);
     unsigned long limit1 = duration1 / 2;
     int extender1;
     if(limit1 <= 300){
      extender1 = limit1;
     }
     else{
      extender1 = 300;
     }
     unsigned long start_offset1 = (start_time1 + limit1);
     unsigned long end_offset1 = (end_time1 + extender1);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date1[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time1 <= current_seconds) && (current_seconds <= start_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time1 <= current_seconds) && (current_seconds <= end_offset1)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }

     // schedules 3 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/dayStamp")){
  
      date2 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/startStamp")){
  
      start_time2 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/2/endStamp")){
  
      end_time2 = firebaseData.intData();
      
     }

     unsigned long duration2 = (end_time2 - start_time2);
     unsigned long limit2 = duration2 / 2;
     int extender2;
     if(limit2 <= 300){
      extender2 = limit2;
     }
     else{
      extender2 = 300;
     }
     unsigned long start_offset2 = (start_time2 + limit2);
     unsigned long end_offset2 = (end_time2 + extender2);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date2[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time2 <= current_seconds) && (current_seconds <= start_offset2)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time2 <= current_seconds) && (current_seconds <= end_offset2)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }

     // schedules 4 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/3/dayStamp")){
  
      date3 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/3/startStamp")){
  
      start_time3 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/3/endStamp")){
  
      end_time3 = firebaseData.intData();
      
     }

     unsigned long duration3 = (end_time3 - start_time3);
     unsigned long limit3 = duration3 / 2;
     int extender3;
     if(limit3 <= 300){
      extender3 = limit3;
     }
     else{
      extender3 = 300;
     }
     unsigned long start_offset3 = (start_time3 + limit3);
     unsigned long end_offset3 = (end_time3 + extender3);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date3[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time3 <= current_seconds) && (current_seconds <= start_offset3)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time3 <= current_seconds) && (current_seconds <= end_offset3)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }

     // schedules 5 //
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/4/dayStamp")){
  
      date4 = firebaseData.stringData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/4/startStamp")){
  
      start_time4 = firebaseData.intData();
      
     }
  
     if(Firebase.get(firebaseData, "/IP210505/schedules/4/endStamp")){
  
      end_time4 = firebaseData.intData();
      
     }

     unsigned long duration4 = (end_time4 - start_time4);
     unsigned long limit4 = duration4 / 2;
     int extender4;
     if(limit4 <= 300){
      extender4 = limit4;
     }
     else{
      extender4 = 300;
     }
     unsigned long start_offset4 = (start_time4 + limit4);
     unsigned long end_offset4 = (end_time4 + extender4);
  
     while(!timeClient.update()){
      timeClient.forceUpdate();
     }
  
     currentDay = timeClient.getDay();
  
     for(int i = 0; i < 7; i++){
      if(currentDay == i){ 
       if(date4[i] == '1'){
        
         while(!timeClient.update()){
          timeClient.forceUpdate();
         }
  
         currentHour = timeClient.getHours();
         currentMin = timeClient.getMinutes();
         currentSec = timeClient.getSeconds();
  
         current_seconds = ((currentHour * 60 * 60) + (currentMin * 60) + (currentSec)); 
   
         if((start_time4 <= current_seconds) && (current_seconds <= start_offset4)){
          Firebase.setString(firebaseData, "/IP210505/uv", "ON");         
         }
         if((end_time4 <= current_seconds) && (current_seconds <= end_offset4)){
          Firebase.setString(firebaseData, "/IP210505/uv", "OFF"); 
         }                      
       }
      }
     }
            
    }

  }
      
}

void loop(){
  
}

void connection() {

 timer1 = millis();
 
  while(1){
   
    if (millis() - timer1 >= 500UL){
      wifi_sta_list_t wifi_sta_list;
      tcpip_adapter_sta_list_t adapter_sta_list;
     
      memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
      memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
     
      esp_wifi_ap_get_sta_list(&wifi_sta_list);
      tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
     
      for (int i = 0; i < adapter_sta_list.num; i++) {
     
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        host = ip4addr_ntoa(&(station.ip));
        Serial.println(host);
        if (client.connect(host, port1)) {
         return;
        }    
      }
     
      timer1 = millis();
      }    
   }
}

void Credential_listener(){

 byte flag1 = 0;
 
  while(1){
   
      while(client.connected() < 1){
       connection();
      }

      while(client.available() > 0){
       line = client.readStringUntil('\r');
       len = line.length();
       o = len;
       flag1++;
       if(flag1 == 1){
        l = 0;
        len = (len + 0);
       }
       else if(flag1 == 2){
        l = 32;
        len = (len + 32);
       }

         if ((line.length() > 0) && (flag1 == 1)) {
          Serial.println("clearing eeprom");
          for (int k = 0; k < 500; ++k) {
            EEPROM.write(k, 0);
          }
          EEPROM.commit();
         }

          for((l, j = 0); (l < len) && (j < o); (++l, ++j)){
           EEPROM.write(l, line[j]);
          }
          EEPROM.commit();
          while(flag1 == 2){
           Serial.println("Writing eeprom completed!!!");
           flag1 = 0;
           for(int i = 0; i<512; i++){
             byte a = EEPROM.read(i);
             Serial.println(a);
           }
           ESP.restart();
          }

      }
     
   }  
 
}
