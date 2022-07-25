
#include <espnow.h>
#include <ESP8266WiFi.h>

// variables
double dur = 0;
double dis = 0;
int Dline = 0;
int Aline = 0;
int n = 0;
bool ACCIDENT_state = false;      //false means we move "all clear" , true means we stop
bool TRAFFIC_state = false;
int street_state;

//pins
int PWM_R = 5;     
int PWM_L = 4;     
int Dir_R = 0;     
int Dir_L = 2;     
int IR_sens = 14;
int minSpeed = 125;
int noSpeed = 0;
int US_trig = 12;
int US_echo = 13;

//network stuff
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
int isstreetbusy = 0;                //0 --> clear , 1 --> busy
int* ptr_isstreetbusy = &isstreetbusy;

typedef struct packet {
    int report;                      //report = 1 --> announce to make street busy , 0 --> announce to make street clear
} packet;

packet outgoingMessage;
packet incomingMessage;

void forward(){

  analogWrite(PWM_L, 140); 
  analogWrite(PWM_R, 140);
  digitalWrite(Dir_R, LOW); 
  digitalWrite(Dir_L, LOW); 
  
}

void sstop(){
  digitalWrite(PWM_R, noSpeed); 
  digitalWrite(Dir_R, LOW);   
  digitalWrite(PWM_L, noSpeed); 
  digitalWrite(Dir_L, LOW); 
  
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("From OnDataSent() : Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Serial.print("Bytes received: ");
  Serial.println(len);
  *ptr_isstreetbusy = incomingMessage.report;
  Serial.print("From OnDataRecv() : isstreetbusy: ");
  Serial.println(*ptr_isstreetbusy);
}
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);     // the function called when send
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);   //add peers
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);     
  esp_now_register_recv_cb(OnDataRecv);    // the function called when receive
  
  // Configure the pins
  pinMode(PWM_R, OUTPUT); 
  pinMode(PWM_L, OUTPUT); 
  pinMode(Dir_R, OUTPUT); 
  pinMode(Dir_L, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(IR_sens, INPUT); 
  pinMode(US_trig, OUTPUT); 
  pinMode(US_echo, INPUT); 
  
  delay(100);
  //moving the vehicle
  forward();
  
}


 
void loop() {

/*ACCIDENT_state = accidentCheck();
  if(ACCIDENT_state){
      return;        //restart the void loop() untill the car infront of us move
  }*/
  
  //delay(100);
  TRAFFIC_state = IntersectionChecK();    //True if the car crossed the line
  if(TRAFFIC_state)
  {
      if(n == 0)          //n = 0 is the beginning of the intersection so we stop and announce that street is busy
      {
          n = (n + 1)%2;
          while(true)    //if the street busy , keep looping untill it is clear
          {
              street_state = WhatIsMy_isstreetbusy();
              if(street_state == 0)
              {
                  delay(2000);
                  delay(random(100 , 1500));                //random delay in case of more than one car are waiting to pass
                  street_state = WhatIsMy_isstreetbusy();
                  if(street_state == 1){continue;}           //by random , one of them will make the street busy
                  AskToPass();
                  forward();
                  break;
              }
          }
          delay(500);
      }
      else               //n = 1 is the end of intersection so we continue and announce that street is clear
      {
          forward();
          EndOfPass();
          n = (n + 1)%2;
      }
  }
  //delay(100);
  
}

int WhatIsMy_isstreetbusy(){
  return *ptr_isstreetbusy;
}

void AskToPass(){
  
    outgoingMessage.report = 1;     //announce busy street
    esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    delay(100);
    Serial.println("From AskToPass() : Moving the car");
    forward(); 
    
}

void EndOfPass(){
  
    outgoingMessage.report = 0;     //announce clear street
    esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    Serial.println("Printing From EndOfPass() : Second BlackLine , Stopping the car , End of the journey");
    delay(1500);
    sstop();
}

bool IntersectionChecK(){
  Aline = analogRead(A0);
  Dline = digitalRead(IR_sens);      // 0 --> White street    , 1 --> Black line
  if(Dline == 1){   
      //stop the car
      Serial.println("Printing From IntersectionChecK() : BlackLine , Stopping the car");
      sstop(); 
      return true;
  }
  return false;
}



bool accidentCheck(){

  digitalWrite(US_trig, LOW);
  delayMicroseconds(5);
  digitalWrite(US_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_trig, LOW);

  dur = pulseIn(US_echo, HIGH);  //in microsec
  dis = (dur/2) / 29.1;  // dis = t*v ---> v =343 m/s = 0.0343 cm/us = 1/29.1

  if(dis < 10){
      //stop the car
      Serial.println("Accident");
      digitalWrite(PWM_R, LOW); 
      digitalWrite(PWM_L, LOW); 
      return true;  
  }
  return false;
}
