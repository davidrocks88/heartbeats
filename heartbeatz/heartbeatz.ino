/*
 * heartbeatz.ino
 * David Bernstein, Gabriella Bova, Chase Conley, Flora Liu, Dani Kupfer
 * Last Updated: May 6, 2016
 *
 * Purpose: 
 *          - Receives a signal from a lightblue bean that is a hearbeat signal 
 *          - prints to the serial port the average bpm for the given signal
 * Usage: 
 *        - Compile and upload the program to a bean via arduino bean loader
 *        - Connect the HEART pin to the signal pin for the heartbeat sensor
 *        - Connect the PLAY_PAUSE pin to the PLAY_PAUSE button
 *        - Connect the SKIP pin to the skip button
 *        - Interface bean's serial port with another device to process bpm data
 */

/******************************************************************************
 *                                Interface                                   *
 ******************************************************************************/

#include <PinChangeInt.h>
#include <QueueList.h>

/*
 * Redefinitions 
 *   - Redefining ulong to make code more concise
 */
#define ulong unsigned long

/* 
 * Function Prototypes
 *   - beat() => called for every beat and logs out the average BPM sensed
 *   - queueAvg() => returns the average of the global queue at any given time
 *   - skip() and play() => outputs appropriate message when button is pressed
 *   - threshold() => transforms a bpm to a threshold code of 0 through 5
 */
void beat();
ulong queueAvg();
void play();
void skip();
int threshold(ulong bpm);
/* 
 * Constants
 *   - SEC_PER_MIN => number of seconds in a minute
 *   - MICROSEC_PER_SEC => number of microseconds in a full second
 *   - NUMBEATS => number of beats to take running average over
 *   - HEART => pin to be used to sense the heartbeat signal
 *   - PLAYPAUSE_BUTTON => pin to be used for play/pause button
 *   - SKIP => pin to be used for skip button
 */
const ulong SEC_PER_MIN = 60;
const ulong MICROSEC_PER_SEC = 1000000;
const int NUMBEATS = 10;
const int HEART = 2;
const int PLAYPAUSE_BUTTON = 4;
const int SKIP_BUTTON = 5;
const char* SKIP_MESSAGE = "6";
const char* PLAY_MESSAGE = "7";
const char* PAUSE_MESSAGE = "8";

/*
 * Global variables
 *   - start => starting time for each heart beat
 *   - ended => ending time for each heart beat
 *   - beatQueue => holds the times between the NUMBEATS most recent beats
 *   - playpause => decides if pressing the play button plays or pauses
 *   - start_pp and start_skip => timer for whether or not to log a message
 *   - no_beats => variable for base case of having no beats yet
 */
ulong start = 0;
ulong ended = 0;
ulong start_pp = 0;
ulong start_skip = 0;
int no_beats = 0;
QueueList <ulong> beatQueue;
bool playpause = true;



/******************************************************************************
 *                              Implementation                                *
 ******************************************************************************/

/* 
 * setup()
 *   - Sets up the heart pin so that beat() is called for every beat sensed
 *   - Sets up play and skip to go to their appropriate functions
 *   - Configures serial for bean and queue at 9600 bps
 */
void setup() {
  pinMode(PLAYPAUSE_BUTTON, INPUT_PULLUP);
  pinMode(SKIP_BUTTON, INPUT_PULLUP);
  pinMode(HEART, INPUT_PULLUP);

  attachPinChangeInterrupt(PLAYPAUSE_BUTTON, play, FALLING);
  attachPinChangeInterrupt(SKIP_BUTTON, skip, FALLING);
  attachPinChangeInterrupt(HEART, beat, FALLING);

  Serial.begin (57600); // 9600 bps
  beatQueue.setPrinter(Serial);
}

/* 
 * loop() 
 *   - does nothing, the program is inactive without any heartbeat signal
 */
void loop() {
}

/*
 * play()
 *   - called when play button is pressed
 *   - sends the PLAY_MESSAGE or PAUSE_MESSAGE to the serial port if a 
 *     second has passed since the last message
 *   - changes value of playpause after every function call
 */
void play() {
  ulong end_pp = micros();
  ulong diff = end_pp - start_pp;
  
  if(diff > MICROSEC_PER_SEC/2) {
    if(playpause) {
      Serial.println(PLAY_MESSAGE);
    }
    else {
      Serial.println(PAUSE_MESSAGE);
    }
    start_pp = micros();
    playpause = !playpause;
  }
}

/*
 * skip()
 *   - called when play button is pressed
 *   - sends the SKIP_MESSAGE to the serial port if a second has passed
 *     since the last SKIP_MESSAGE
 */
void skip() {
  ulong end_skip = micros(); 
  ulong diff = end_skip - start_skip;
  
  if(diff > MICROSEC_PER_SEC/2) {
    Serial.println(SKIP_MESSAGE);
    start_skip = micros();
  }
}

/* 
 * beat()
 *   - prints the average bpm from the 20 most recent bpm measurements
 *   - if only one beat has been detected so far, then nothing is outputted
 *   - calculates the time it took to reach the current beat, then calls 
 *     queueAvg() to get the average bpm to be outputted.
 *   - if there are more than NUMBEATS beats in the queue, then it will pop
 *     from the queue so that one more beat can be added, restoring the count
 *     of the queue back to NUMBEATS
 */
void beat() {
  ulong average = 0;
  ulong diff = 0;
   
 if(no_beats++ == 0) {
   start = micros();
   return;
 }
 else {
    ended = micros();
    diff = ended - start; 
    start = micros();
 }
   
 if(beatQueue.count() >= NUMBEATS) {
   beatQueue.pop();
 }
   
 beatQueue.push(diff);
 
 ulong bpm = SEC_PER_MIN * MICROSEC_PER_SEC / queueAvg();
 
 if(beatQueue.count() >= 10) 
   Serial.println(threshold(bpm));
   //Serial.println(bpm);
}

/*
 * queueAvg() 
 *   - pops and pushes all the values of the queue and returns the average
 */
ulong queueAvg() {
  ulong avg = 0;
  
  for(int i = 0; i < beatQueue.count(); i++) {
    ulong temp = beatQueue.pop();
    avg += temp;
    beatQueue.push(temp);    
  }
  
  return avg / beatQueue.count(); 
}

/*
 * threshold(bpm)
 * returns the appropriate threshold code according to the given bpm value
 */
int threshold(ulong bpm) {
  if(bpm <= 90) return 1;
  else if(bpm <= 110) return 2;
  else if(bpm <= 130) return 3;
  else if(bpm <= 150) return 4;
  else return 5;
}
