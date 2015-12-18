// HitchHiker Laboratories Abattoir
//
// Abattoir by Sylvain Poitras is licensed under
// the GNU General Public License v.3
// https://www.gnu.org/copyleft/gpl.html
//
// Looping engine code adapted from Glenn Mossy's sequence recorder at 
// http://littlebits.cc/projects/sequence-recorder
// licenced under Creative Commons CC-BY-SA
// 
// Iterrupt setup code from: 
// http://playground.arduino.cc/Main/PinChangeInterrupt

#include <Metro.h>

const byte GATE1_ON = 11;
const byte GATE1_OFF = 10;
const byte RED = 8;
const byte GREEN = 7;
const byte VOICELEDS = 9;
const byte VOICE1 = 2;
const byte REC = 3;
const byte SPEED1 = A0;
const byte MK1 = 6;
int counter =0;

Metro reset = Metro(500);
Metro loop1Metro = Metro(0);
Metro playback1Metro = Metro(0);

// buffers to record state/duration combinations
int maxSamples = 300;   // This is the maximum number of samples in bytes that can be stored.
boolean states[300];    // Create a states array to record up to this number of on and off states.
unsigned int durations[300];     // Create a durations array and we can have up to this many duration times.

volatile int idxPlayback  = 0;   // Initialize the index for the Playback array to 0
volatile int idxRecord    = 0;   // Initialize the index for the Record array to 1
int sampleLength = 2;   // 2 ms
volatile float playbackRate1 = 1;   // 2=>half SPEED1, 0.5=>double SPEED1, change this if you want to play back slower or faster than the recorded rate.
volatile float recordRate1=1;
volatile boolean voice1ON = false;
volatile boolean recON = false;

volatile boolean brk_mk_1 = true;

enum loopStatus {
  armed,
  recording,
  playing,
  paused,
  emptyLoop};

volatile loopStatus loopState = emptyLoop;

volatile unsigned long lastVoice1 =0;
volatile unsigned long lastRec =0;

volatile unsigned long seqDuration=0;
volatile unsigned long startMS;

unsigned long loopMS=0;

// Interrupt setup

void pciSetup(byte pin)
{
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

// Interrupt Service Routine (ISR)
ISR (PCINT2_vect) 
{
  // react to voice1 release
  if (digitalRead (VOICE1) == LOW && voice1ON==true){

    voice1ON = false;
    brk_mk_1=digitalRead(MK1);
    voice1AR(!brk_mk_1);
  }

  // react to voice1 press
  if(digitalRead(VOICE1)==HIGH && voice1ON==false){
    //      Serial.print("doing 1");
    voice1ON = true;
    brk_mk_1=digitalRead(MK1);
    voice1AR(brk_mk_1);
    
    if(loopState==armed){
      float dimmerVal = 1023-analogRead(SPEED1); // wired upside down!              
      recordRate1 = (dimmerVal/1023 *1.5)+0.5;
      playbackRate1 = recordRate1;

      loopState=recording;
      recordingLED();
      memset(states, 0, sizeof(states));        // Set the size of the states array
      memset(durations, 0, sizeof(durations));  // Set the size of the durations array
      idxRecord = 0;     // reset record idx just to make playback start point obvious
      return;
    }
  }

  // react to REC release
  if (digitalRead (REC) == LOW && recON==true){
    recON = false;

  }

  // react to REC press
  if(digitalRead(REC)==HIGH && recON==false){
    recON = true;
    reset.reset();
    //            Serial.print("off");

    if(loopState==recording){
      seqDuration=0;
      for(int i=0;i<idxRecord+1;i++){
        seqDuration+=durations[i];
      }

      loop1Metro.interval(0);
      loopState=playing;
      playingLED();  
      return;
    }

    if(loopState==playing){
      pauseLED();  
       brk_mk_1=digitalRead(MK1);      
      voice1AR(!brk_mk_1);

      loopState=paused;
      return;
    }

    if(loopState==paused){
      playingLED();
      playback1Metro.reset();
      playback1Metro.interval(0);
      idxPlayback=0;
      loopState=playing;
      return;
    }

    if(loopState==emptyLoop){
      loopState=armed;
      recordingLED();
      return;
    } 
  }  
}

// LED helper functions
void playingLED(){
  digitalWrite(RED,LOW);
  digitalWrite(GREEN,HIGH);
}

void recordingLED(){
  digitalWrite(RED,HIGH);
  digitalWrite(GREEN,LOW);
}

void pauseLED(){
  digitalWrite(RED,HIGH);
  digitalWrite(GREEN,HIGH);
}

void offLED(){
  digitalWrite(RED,LOW);
  digitalWrite(GREEN,LOW);
}

// setup
void setup ()
{
//  buttons
pinMode(VOICE1, INPUT);
pinMode(REC, INPUT);

pciSetup(VOICE1);
pciSetup(REC);

//  VTL5C3 LEDS
  pinMode(GATE1_ON, OUTPUT);
  pinMode(GATE1_OFF,OUTPUT);  
//  Feedback LEDS

  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  pinMode(VOICELEDS,OUTPUT);  
  
  
  offLED();
  
//  Potentiometers for playback speed
  pinMode(SPEED1,INPUT_PULLUP);
  
// Switches for brk / mk
  pinMode(MK1, INPUT_PULLUP);
  
// initial state- gate off
  brk_mk_1=digitalRead(MK1);
  voice1AR(!brk_mk_1);    

}  // end of setup

// main loop
void loop ()
{   
  if(recON==true && reset.check()==true){
    loopState=emptyLoop;
    memset(states, 0, sizeof(states));        // Set the size of the states array
    memset(durations, 0, sizeof(durations));  // Set the size of the durations array
    idxRecord = 0;     // reset record idx just to make playback start point obvious
    loop1Metro.reset();
    loop1Metro.interval(0);
    brk_mk_1=digitalRead(MK1);
    voice1AR(!brk_mk_1);

    offLED();
    
  }
  if(brk_mk_1 != digitalRead(MK1)){
    brk_mk_1 = digitalRead(MK1);
    voice1AR(!brk_mk_1);
  }
  
   if(loopState==recording) {                          
    recordLoop();        // perform the recording loop function
  }
  if(loopState==playing){
    playbackLoop();      // perform the playback loop function
  }
}

// playback loop
void playbackLoop()
{
  if(loop1Metro.check()==true){
    loop1Metro.reset();
    idxPlayback=0;
    float dimmerVal = 1023-analogRead(SPEED1); // wired upside down!             
    playbackRate1 = (dimmerVal/1023 *1.5)+0.5;
    loop1Metro.interval(seqDuration*playbackRate1/recordRate1);

    playback1Metro.interval(0);
    playback1Metro.reset();
    
  }
 
  
  if(playback1Metro.check()==true){
    playback1Metro.reset();
    float dimmerVal = 1023-analogRead(SPEED1); // wired upside down!             
    playbackRate1 = (dimmerVal/1023 *1.5)+0.5;
    loop1Metro.interval(seqDuration*playbackRate1/recordRate1);
    // play the loop back at the desired SPEED1
    brk_mk_1=digitalRead(MK1);
    if(states[idxPlayback]){
      voice1AR(brk_mk_1);
    }else{
      voice1AR(!brk_mk_1);
    }
    
    playback1Metro.interval(durations[idxPlayback] * playbackRate1/recordRate1);  
    idxPlayback++;                                   // increment the playback index so we can play the next value
    if(idxPlayback == maxSamples) { idxPlayback=0; } // repeat the recorded loop when we reach the maximum number of samples
  }
}

// record loop
void recordLoop() {
  boolean state = voice1ON; // read the state of the input sequence recorder button.  

  if(states[idxRecord] == state) {
    // if the state is not changed, add to duration of current state
    durations[idxRecord] += sampleLength;
  } else {
    // if the state is changed, go to next index (idx) and set default duration of 2ms
    idxRecord++;
    if(idxRecord == maxSamples) { idxRecord = 0; } // reset idx if max array size reached
    states[idxRecord] = state;                     // save the state to the states array by index
    durations[idxRecord] = sampleLength;           // save the duration of the sample to the durations array by index
  }
  
  delay(sampleLength); // slow the loop to a time constant so we can reproduce the timeliness of the recording
}

// voice1 helper function
void voice1AR(boolean state){
  digitalWrite(GATE1_OFF,!state);
  digitalWrite(GATE1_ON,state);
  
  digitalWrite(VOICELEDS,state);
}