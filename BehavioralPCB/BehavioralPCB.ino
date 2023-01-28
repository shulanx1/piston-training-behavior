/* Read Quadrature Encoder
   Connect Encoder to Pins encoder0PinA, encoder0PinB, and +5V.

*/
//print sequence: rundist, lick, valve, pistonGO, pisktonNOGO, time

#include <arduino2.h>

int val;
const int lick_sensor = 2;
const int valve_enable = 3;
const int pistonGO = 8;
const int pistonNOGO = 9;
int encoder0PinA = 6;
int encoder0PinB = 5;
int encoder0Pos = 0;
long int encoder0PosTot = 0;
int encoder0PinALast = LOW;
int n = LOW;
int count = 0;
int counter = 250;
int LED1 = A1;
int trigger = 10;
int lick_state = 0;
int valve_state = 0;
int previousState = LOW;
int LED2 = A2;
int flag = 0;
int respond_window = 1000;  // time window (ms) after sensory stimuli that if lick is detected reward will be given
int reward_dur = 50;   // duration (ms) of reward
int stim_dur = 1000; // time duration (ms) of sensory stimuli
unsigned long millisStart;
unsigned long millisFix;
unsigned long deltaT;
float convertPos;
void setup() {
  Serial.begin(9600);  
  while (!Serial) {
    //wait for serial port to open
  }
  pinMode (encoder0PinA, INPUT);
  pinMode (encoder0PinB, INPUT);
  pinMode (lick_sensor, INPUT);
  pinMode (valve_enable, OUTPUT);
  pinMode (pistonGO, OUTPUT);
  pinMode (pistonNOGO, OUTPUT);
  pinMode (trigger, INPUT);
  pinMode (LED1, OUTPUT);
  pinMode(LED2,OUTPUT);
  digitalWrite2(valve_enable,LOW);
  digitalWrite2(pistonGO, LOW);
  digitalWrite2(pistonNOGO, LOW);
  previousState = digitalRead2(trigger);
  Serial.begin (9600);
  millisStart=millis();
  flag = 0;
  Serial.print("Ready");
}

void loop() {
  if((previousState == LOW) && (digitalRead2(trigger) == HIGH)){
    flag = 1;
    digitalWrite2(LED2,HIGH);
    millisFix = millis();
    previousState = HIGH;
  }
  else if((previousState == HIGH) && (digitalRead2(trigger) == LOW)){
      flag = 0;
      digitalWrite2(LED2,LOW);
      convertPos = 0;
      encoder0PosTot = 0;
      previousState = LOW;
  }

  if (flag == 1){
    char a = Serial.read();
    switch(a)
    {
      case '0':   // reward given whenever lick happens
        if (digitalRead2(lick_sensor) == HIGH){
          lick_state = 1;
          valve_state = 1;
          digitalWrite2(valve_enable, HIGH);
          delay(reward_dur);
          digitalWrite2(valve_enable, LOW);
          deltaT = millis()-millisFix;
          Serial.print(convertPos);
          Serial.print("\t");
          Serial.print(lick_state);
          Serial.print("\t");
          Serial.print(valve_state);
          Serial.print("\t");
          Serial.print(0);
          Serial.print("\t");
          Serial.print(0);
          Serial.print("\t");
          Serial.println(deltaT/1000.00);
        }
        lick_state = 0;
        valve_state = 0;
        break;

      case '1':   // GO stimuli, reward given regardless of licking
        count = 0;
        digitalWrite2(pistonGO, HIGH);
        delay(stim_dur);
        digitalWrite2(pistonGO, LOW);
        digitalWrite2(valve_enable, HIGH);
        while (count < reward_dur){
          if (digitalRead2(lick_sensor)==HIGH){
            lick_state = 1;
          }
          delay(1);
          count = count + 1;
        }
        digitalWrite2(valve_enable, LOW);
        valve_state = 1;
        deltaT = millis()-millisFix;
        Serial.print(convertPos);
        Serial.print("\t");
        Serial.print(lick_state);
        Serial.print("\t");
        Serial.print(valve_state);
        Serial.print("\t");
        Serial.print(1);
        Serial.print("\t");
        Serial.print(0);
        Serial.print("\t");
        Serial.println(deltaT/1000.00);
        lick_state = 0;
        valve_state = 0;
        break;
      
      case '2':   // GO stimuli, reward given only when hit
        digitalWrite2(pistonGO, HIGH);
        count = 0;
        delay(10); // time for piston response
        while (count < stim_dur-10){
          if (digitalRead2(lick_sensor)==HIGH){
            lick_state = 1;
          }
          delay(1);
          count = count + 1;
        }
        //delay(stim_dur);
        digitalWrite2(pistonGO, LOW);
        count = 0;
        while (count < respond_window){
          if (lick_state == 1){
            break;
          }
          else{
            if (digitalRead2(lick_sensor) == HIGH){
              lick_state = 1;
              break;
            }
            delay(1);
            count = count + 1;  
          }        
        }
        if (lick_state == 1){
          digitalWrite2(valve_enable, HIGH);
          delay(reward_dur);
          digitalWrite2(valve_enable, LOW);
          valve_state = 1;
        }
        deltaT = millis()-millisFix;
        Serial.print(convertPos);
        Serial.print("\t");
        Serial.print(lick_state);
        Serial.print("\t");
        Serial.print(valve_state);
        Serial.print("\t");
        Serial.print(1);
        Serial.print("\t");
        Serial.print(0);
        Serial.print("\t");
        Serial.println(deltaT/1000.00);
        valve_state = 0;
        lick_state = 0;
        break;
        
      case '3':   // NOGO stim, 10s time out if FA
        digitalWrite2(pistonNOGO, HIGH);
        //delay(stim_dur);
        count = 0;
        delay(10); // time for piston response
        while (count < stim_dur-10){
          if (digitalRead2(lick_sensor)==HIGH){
            lick_state = 1;
          }
          delay(1);
          count = count + 1;
        }
        //delay(stim_dur);
        digitalWrite2(pistonNOGO, LOW);
        count = 0;
        while (count < respond_window){
          if (lick_state == 1){
            break;
          }
          else{
            if (digitalRead2(lick_sensor) == HIGH){
              lick_state = 1;
              break;
            }
            delay(1);
            count = count + 1;  
          }        
        }
        if (lick_state == 1){
          delay(20000); //time out
        }
        deltaT = millis()-millisFix;
        Serial.print(convertPos);
        Serial.print("\t");
        Serial.print(lick_state);
        Serial.print("\t");
        Serial.print(valve_state);
        Serial.print("\t");
        Serial.print(0);
        Serial.print("\t");
        Serial.print(1);
        Serial.print("\t");
        Serial.println(deltaT/1000.00);
        valve_state = 0;
        lick_state = 0;
        break; 

      case '7':   // NOGO stim, no reward, no time out
        digitalWrite2(pistonNOGO, HIGH);
        //delay(stim_dur);
        count = 0;
        delay(10); // time for piston response
        while (count < stim_dur-10){
          if (digitalRead2(lick_sensor)==HIGH){
            lick_state = 1;
          }
          delay(1);
          count = count + 1;
        }
        //delay(stim_dur);
        digitalWrite2(pistonNOGO, LOW);
        count = 0;
        while (count < respond_window){
          if (lick_state == 1){
            break;
          }
          else{
            if (digitalRead2(lick_sensor) == HIGH){
              lick_state = 1;
              break;
            }
            delay(1);
            count = count + 1;  
          }        
        }
        deltaT = millis()-millisFix;
        Serial.print(convertPos);
        Serial.print("\t");
        Serial.print(lick_state);
        Serial.print("\t");
        Serial.print(valve_state);
        Serial.print("\t");
        Serial.print(0);
        Serial.print("\t");
        Serial.print(1);
        Serial.print("\t");
        Serial.println(deltaT/1000.00);
        valve_state = 0;
        lick_state = 0;
        break; 

      case '4':   // test water reward
        count = 0;
        while (count < 5){
          digitalWrite2(valve_enable, HIGH);
          delay(reward_dur);
          digitalWrite2(valve_enable, LOW);
          delay(500);
          count = count +1;
          }
        count = 0;
        lick_state = 0;
        valve_state = 0;
        break;  

        
      case '5':   // test piston
        count = 0;
        while (count < 10){
          digitalWrite2(pistonGO, HIGH);
          delay(stim_dur);
          digitalWrite2(pistonGO, LOW);
          delay(500);
          digitalWrite2(pistonNOGO, HIGH);
          delay(stim_dur);
          digitalWrite2(pistonNOGO, LOW);
          delay(500);
          count = count +1;
        }
        digitalWrite2(pistonGO, LOW);
        digitalWrite2(pistonNOGO, LOW);
        count = 0;
        lick_state = 0;
        valve_state = 0;
        break; 
/*
      case '7':   //drain the water
        digitalWrite2(valve_enable, HIGH);
        delay(10000);
        digitalWrite2(valve_enable, LOW);
        break;

        */
    }
        
    n = digitalRead2(encoder0PinA);
    if ((encoder0PinALast == LOW) && (n == HIGH)) {
      analogWrite(LED1,128);
      if (digitalRead2(encoder0PinB) == LOW) {
        encoder0Pos++;
        encoder0PosTot++;
      } else {
        encoder0Pos--;
        encoder0PosTot--;
      }
    }   
    /*
    else if ((encoder0PinALast == HIGH) && (n == LOW)){
      analogWrite(LED1,128);
      if (digitalRead2(encoder0PinB) == HIGH) {
        encoder0Pos++;
        encoder0PosTot++;
      } else {
        encoder0Pos--;
        encoder0PosTot--;
      }    
    } 
    */  
      //Counter for reward dispension
    convertPos = (2*3*3.141*2.54*encoder0PosTot)/1000.00;
    if (digitalRead2(lick_sensor) == HIGH) {
      lick_state = 1;
    }
    deltaT = millis()-millisFix;
    Serial.print(convertPos);
    Serial.print("\t");
    Serial.print(lick_state);
    Serial.print("\t");
    Serial.print(valve_state);
    Serial.print("\t");
    Serial.print(0);
    Serial.print("\t");
    Serial.print(0);
    Serial.print("\t");
    Serial.println(deltaT/1000.00);
    lick_state = 0;
    valve_state = 0;
    delay(10);

    encoder0PinALast = n;
    digitalWrite2(LED1,LOW);
  }
  else{
    char a = Serial.read();
    switch (a){
      case '4':   // test water reward
        count = 0;
        while (count < 5){
          digitalWrite2(valve_enable, HIGH);
          delay(reward_dur);
          digitalWrite2(valve_enable, LOW);
          delay(500);
          count = count +1;
          }
        count = 0;
        lick_state = 0;
        valve_state = 0;
        break;  

        
      case '5':   // test piston
        count = 0;
        while (count < 10){
          digitalWrite2(pistonGO, HIGH);
          delay(stim_dur);
          digitalWrite2(pistonGO, LOW);
          delay(500);
          digitalWrite2(pistonNOGO, HIGH);
          delay(stim_dur);
          digitalWrite2(pistonNOGO, LOW);
          delay(500);
          count = count +1;
        }
        digitalWrite2(pistonGO, LOW);
        digitalWrite2(pistonNOGO, LOW);
        count = 0;
        lick_state = 0;
        valve_state = 0;
        break; 
      
      case '7':   //drain the water
        digitalWrite2(valve_enable, HIGH);
        delay(30000);
        digitalWrite2(valve_enable, LOW);
        break;
    }
      
  }
}
