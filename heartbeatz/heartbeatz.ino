/*
 * heartbeatz.ino
 * David Bernstein, Gabriella Bova, Chase Conley, Flora Liu, Dani Kupfer
 * Last Updated: April 10, 2016
 *
 * Purpose: 
 *          - Receives a signal from a lightblue bean that is a hearbeat signal 
 *          - prints to the serial port the average bpm for the given signal
 * Usage: 
 *        - Compile and upload the program to a bean via arduino bean loader
 *        - Connect the HEART pin to the signal pin for the heartbeat sensor
 *        - Connect the PLAY pin to the PLAY button
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
 */
void beat();
ulong queueAvg();
void play();
void skiph();

/* 
 * Constants
 *   - SEC_PER_MIN => number of seconds in a minute
 *   - MICROSEC_PER_SEC => number of microseconds in a full second
 *   - NUMBEATS => number of beats to take running average over
 *   - HEART => pin to be used to sense the heartbeat signal
 *   - PLAY => pin to be used for play button
 *   - SKIP => pin to be used for skip button
 */
const ulong SEC_PER_MIN = 60;
const ulong MICROSEC_PER_SEC = 1000000;
const int NUMBEATS = 20;
const int HEART = 0;
const int PLAY_BUTTON = 1;
const int SKIP_BUTTON = 2;

/*
 * Global variables
 *   - start => starting time for each heart beat
 *   - ended => ending time for each heart beat
 *   - beatQueue => holds the times between the NUMBEATS most recent beats
 */
ulong start = 0;
ulong ended = 0;
QueueList <ulong> beatQueue;

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
  attachPinChangeInterrupt(HEART, beat, FALLING);
  //attachPinChangeInterrupt(PLAY_BUTTON, play, FALLING);
  attachPinChangeInterrupt(PAUSE_BUTTON, skiph, FALLING);

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
 *   - sends the PLAY_MESSAGE to the serial port
 */
void play() {
  Serial.println("PLAY"); 
}

/*
 * skip()
 *   - called when play button is pressed
 *   - sends the SKIP_MESSAGE to the serial port
 */
void skiph() {
  Serial.println("SKIP"); 
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
   
   if(beatQueue.count() == 0) {
     start = micros();
     return;
   }
   else {
     ended = micros();
     diff = ended - start; 
     start = micros();
   }
   
   if(beatQueue.count() == NUMBEATS) {
     beatQueue.pop();
   }
   
   beatQueue.push(diff);
   
   Serial.println(SEC_PER_MIN * MICROSEC_PER_SEC / queueAvg());
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
  
  return avg / 10; 
}
