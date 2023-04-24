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
//int encoder0Pos = 0;
long int encoder0PosTot = 0;
int encoder0PinALast = LOW;
int n = LOW;
int count = 0;
int counter = 250;
int LED1 = A1;
int audio = A3;
int trigger = 10;
int lick_state = 0;
int valve_state = 0;
int go_stim_state = 0;
int nogo_stim_state = 0;
int go_teach = 0;
int nogo_teach = 0;
int previousState = LOW;
int LED2 = A2;
int flag = 0;
const int respond_window = 1000;  // time window (ms) after sensory stimuli that if lick is detected reward will be given
const int reward_dur = 100;   // duration (ms) of reward
const int stim_dur = 3000; // time duration (ms) of sensory stimuli
const int time_out_dur = 15000;  // time out duration (ms)
const int wait_dur = 500;    //1000-sample time (us)
const int teach_dur = 10000; // small time out after teach signal, if lick happens during the time, considered as hit
const float teach_ratio = 0.2;  //if correct detection/rejection is lower than teach ratio, send a teaching signal
const int previous_state_num = 10; //average the performance of previous numbers of trials to generate teach signals
unsigned int previous_go_performance = 24;//992; //0x03e0
unsigned int previous_nogo_performance = 3;//31;  //0x001f
const unsigned int update_shift = 15360; // 0x3c00, inverse the first 3rd to 6th digit, so the updated value is not the same as before 

const int cue_f = 2000;
const int cue_fs = 20000;
float cue_samples[100];
unsigned int cue_interval = 1000000/cue_fs;

int sampEncoderPos = 0;
int sampEncoderPosPrev = 0;
int changePos = 0;
unsigned long sampTime = 0;
int encoder_flag = 0;
float totalPos = 0;
const int fs_encoder = 100; // the sampling frequency of the encoder 

unsigned long microsStart;
unsigned long microsFix;
unsigned long deltaT;
float convertPos;

void setup() {
  Serial.begin(115200);  
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
  pinMode(audio, OUTPUT);
  digitalWrite2(valve_enable,LOW);
  digitalWrite2(pistonGO, LOW);
  digitalWrite2(pistonNOGO, LOW);

  float cue_time = 0.0;
  int cue_cycle = (int) (cue_fs/cue_f);
  for (int n = 0; n < cue_cycle; n++){
    cue_time = (float) n/cue_cycle;
    cue_samples[n] = (float)(127.0*sin(2*3.14*cue_time)+127.0);
  }  

  cli(); // stop interrupts
  
  // set TIMER1 interrupt at 2kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 0.5kHz increments
  OCR1A  = 19999; //(16*10^6) / (fs_encoder*8) - 1 //(must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 8 prescaler
  TCCR1B |= (1 << CS11) | (0 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A); 

  sei(); // allow interrupts

  previousState = digitalRead2(trigger);
  microsStart=micros();
  flag = 0;
  Serial.print("Ready");
}

void loop() {
  if((previousState == LOW) && (digitalRead2(trigger) == HIGH)){
    flag = 1;
    digitalWrite2(LED2,HIGH);
    microsFix = micros();
    previousState = HIGH;
  }
  else if((previousState == HIGH) && (digitalRead2(trigger) == LOW)){
      flag = 0;
      digitalWrite2(LED2,LOW);
      convertPos = 0;
      totalPos = 0;
      encoder0PosTot = 0;
      previousState = LOW;
  }

  if (flag == 1){
    if (Serial.available() > 0){
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
            rotary_decode();
          }
          lick_state = 0;
          valve_state = 0;
          break;

        case '1':   
        /* GO stimuli, 
        if the average mistake in the previous 10 trials larger than 0.5, give reward regardless of licking, 
        otherwise, reward only when licking */
          count = 0;
          digitalWrite2(pistonGO, HIGH);
          go_stim_state = 1;
          count = 0;
          delay(10); // time for piston response
          while (count < stim_dur-10){
            if (digitalRead2(lick_sensor)==HIGH){
              lick_state = 1;
            }
            rotary_decode();
            delayMicroseconds(wait_dur);
            count = count + 1;
          }
          digitalWrite2(pistonGO, LOW);         
          previous_go_performance = ((previous_go_performance << 1) ^ update_shift) & 16383; // left shift, inverse 3rd to 6th digit, zero 1st &2nd digit
          if (lick_state == 1){
            previous_go_performance = previous_go_performance | 1;
          }
          go_teach_signal();
          if ((go_teach == 1) || (lick_state == 1)){
            valve_state = 1;
            digitalWrite2(valve_enable, HIGH);
            delay(reward_dur);
            digitalWrite2(valve_enable, LOW);
            if (go_teach == 1){
              previous_go_performance = previous_go_performance | (1 << 15);
              while (count < teach_dur){ // small time out session after teach signal
                if (digitalRead2(lick_sensor)==HIGH){
                  lick_state = 1;
                  previous_go_performance = previous_go_performance | (1 << 0);
                }
              rotary_decode();
              delayMicroseconds(wait_dur);
              count = count + 1;
              }
            }
          }          
        rotary_decode(); 


        lick_state = 0;
        valve_state = 0;
        go_stim_state = 0;
        break;
        
        case '7':   
        /* NOGO stimuli, 
        if the average mistake in the previous 10 trials larger than 0.5, no time out regardless of licking, 
        otherwise, timeout when licking */
          digitalWrite2(pistonNOGO, HIGH);
          //delay(stim_dur);
          nogo_stim_state = 1;
          count = 0;
          delay(10); // time for piston response
          while (count < stim_dur-10){
            if (digitalRead2(lick_sensor)==HIGH){
              rotary_decode();
              lick_state = 1;
            }
            rotary_decode();
            delayMicroseconds(wait_dur);
            count = count + 1;
          }
          //delay(stim_dur);
          digitalWrite2(pistonNOGO, LOW);
          count = 0;
          while (count < respond_window){
            if (lick_state == 1){
              rotary_decode();
              break;
            }
            else{
              if (digitalRead2(lick_sensor) == HIGH){
                rotary_decode();
                lick_state = 1;
                break;
              }
              rotary_decode();
              delayMicroseconds(wait_dur);
              count = count + 1;  
            }        
          }

          previous_nogo_performance = ((previous_nogo_performance << 1) ^ update_shift) & 16383;
          if (lick_state == 1){
            previous_nogo_performance = previous_nogo_performance | 1;
          }
          nogo_teach_signal();
          if (lick_state == 1){
            if (nogo_teach==0) {
              delay(time_out_dur); //time out
              serialFlush();
            }
            else {
              previous_nogo_performance = previous_nogo_performance | (1 << 15);    // change first digit to 1 if there's teaching signal
              delay(teach_dur);
              serialFlush();
            }
          }
          rotary_decode(); 
          valve_state = 0;
          lick_state = 0;
          nogo_stim_state = 0;
          break; 
        
        case '2':   // GO stimuli, reward given only when hit
          digitalWrite2(pistonGO, HIGH);
          go_stim_state = 1;
          count = 0;
          delay(10); // time for piston response
          while (count < stim_dur-10){
            if (digitalRead2(lick_sensor)==HIGH){
              lick_state = 1;
            }
            rotary_decode();
            delayMicroseconds(wait_dur);
            count = count + 1;
          }
          //delay(stim_dur);
          digitalWrite2(pistonGO, LOW);
          count = 0;
          while (count < respond_window){
            if (lick_state == 1){
              rotary_decode();
              break;
            }
            else{
              if (digitalRead2(lick_sensor) == HIGH){
                rotary_decode();
                lick_state = 1;
                break;
              }
              rotary_decode();
              delayMicroseconds(wait_dur);
              count = count + 1;  
            }        
          }
          previous_go_performance = ((previous_go_performance << 1) ^ update_shift) & 16383;
          if (lick_state == 1){
            rotary_decode();
            previous_go_performance = previous_go_performance | 1;          
            digitalWrite2(valve_enable, HIGH);
            delay(reward_dur);
            digitalWrite2(valve_enable, LOW);
            valve_state = 1;
          }

          valve_state = 0;
          lick_state = 0;
          go_stim_state = 0;
          break;
          
        case '3':   // NOGO stim, 10s time out if FA
          digitalWrite2(pistonNOGO, HIGH);
          //delay(stim_dur);
          nogo_stim_state = 1;
          count = 0;
          delay(10); // time for piston response
          while (count < stim_dur-10){
            if (digitalRead2(lick_sensor)==HIGH){
              rotary_decode();
              lick_state = 1;
            }
            rotary_decode();
            delayMicroseconds(wait_dur);
            count = count + 1;
          }
          //delay(stim_dur);
          digitalWrite2(pistonNOGO, LOW);
          count = 0;
          while (count < respond_window){
            if (lick_state == 1){
              rotary_decode();
              break;
            }
            else{
              if (digitalRead2(lick_sensor) == HIGH){
                rotary_decode();
                lick_state = 1;
                break;
              }
              rotary_decode();
              delayMicroseconds(wait_dur);
              count = count + 1;  
            }        
          }
          previous_nogo_performance = ((previous_nogo_performance << 1) ^ update_shift) & 16383;
          if (lick_state == 1){
            previous_nogo_performance = previous_nogo_performance | 1;
            delay(time_out_dur); //time out
            serialFlush();
          }
          nogo_stim_state = 0;
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
          while (count < 5){
            digitalWrite2(pistonGO, HIGH);
            delay(1000);
            digitalWrite2(pistonGO, LOW);
            delay(500);
            digitalWrite2(pistonNOGO, HIGH);
            delay(1000);
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

        case '6': //play the cue
          play_cue();
          break;
      }   
     
      //Counter for reward dispension
    }
    if (digitalRead2(lick_sensor) == HIGH) {
      lick_state = 1;
    }
    rotary_decode();
    lick_state = 0;
    valve_state = 0;
    

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
        while (count < 5){
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
      
      case '6': //play the cue
        play_cue();
        break;
    
    }
      
  }
}

void rotary_decode(){
    n = digitalRead2(encoder0PinA);
    if ((encoder0PinALast == LOW) && (n == HIGH)) {
      //deltaT = microsStart;
      microsStart = micros()-microsFix;
      digitalWrite2(LED1,HIGH);
      if (digitalRead2(encoder0PinB) == LOW) 
      {
        //encoder0Pos++;
        encoder0PosTot++;
      } else 
      {
        //encoder0Pos--;
        encoder0PosTot--;
      }
    }
    
    else if ((encoder0PinALast == HIGH) && (n == LOW)) {
      //deltaT = microsStart;
      microsStart = micros()-microsFix;
      digitalWrite2(LED1,HIGH);
      if (digitalRead2(encoder0PinB) == HIGH) 
      {
        //encoder0Pos++;
        encoder0PosTot++;
      } else 
      {
        //encoder0Pos--;
        encoder0PosTot--;
      }
    }
    
    encoder0PinALast = n;
    digitalWrite2(LED1,LOW);

  if(encoder_flag == 1){
    changePos = sampEncoderPos - sampEncoderPosPrev;
    convertPos = (2*3*3.141*2.54*changePos)/1000.00;
    totalPos = totalPos + convertPos;
    deltaT = (sampTime-microsFix)/1000.0;
    Serial.print(totalPos);
    Serial.print("\t");
    Serial.print(lick_state);
    Serial.print("\t");
    Serial.print(valve_state);
    Serial.print("\t");
    Serial.print(go_stim_state);
    Serial.print("\t");
    Serial.print(nogo_stim_state);
    Serial.print("\t");
    Serial.print(previous_go_performance);
    Serial.print("\t");
    Serial.print(previous_nogo_performance);
    Serial.print("\t");
    Serial.println(deltaT/1000.00);

    sampEncoderPosPrev = sampEncoderPos;
    encoder_flag = 0;
  }
}


void serialFlush(){
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}

void go_teach_signal(){
  int count = 0;
  boolean c_bit = 0;
  for (int i = previous_state_num-1; i >= 0; i--) {
    c_bit = (previous_go_performance & (1 << i)) != 0;
    if (c_bit == 1){
      count = count + 1;
    }
  }
  if (count <= teach_ratio*previous_state_num) {
    go_teach = 1;
  }
  else {
    go_teach = 0;
  }
}

void nogo_teach_signal(){
  int count = 0;
  boolean c_bit = 0;
  for (int i = previous_state_num-1; i >= 0; i--) {
    c_bit = (previous_nogo_performance & (1 << i)) != 0;
    if (c_bit == 1){
      count = count + 1;
    }
  }
  if (count > teach_ratio*previous_state_num) {
    nogo_teach = 1;
  }
  else {
    nogo_teach = 0;
  }
}

void play_cue(){
  int cue_cycle = (cue_fs/cue_f);
  for (int n = 0; n < (cue_f/2);n++){
    for (int i=0; i < cue_cycle; i++){
      analogWrite(audio, cue_samples[i]);
      delayMicroseconds(cue_interval);

    }
  }
}

ISR(TIMER1_COMPA_vect)
{
  if (flag == 1)
  {
    sampTime = micros();
    sampEncoderPos = encoder0PosTot;
    encoder_flag = 1;
  }
  else{
    sampTime = 0;
    sampEncoderPos = 0;
  }
}