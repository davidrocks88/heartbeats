#include <PinChangeInt.h>
#include <QueueList.h>
#define heart 0
#define numBeats = 20;
unsigned long start = 0;
unsigned long ended = 0;
unsigned long diff = 0;

QueueList <unsigned long> queue;

bool first = true;

unsigned long getAverage();

void setup() {
  // put your setup code here, to run once:
  attachPinChangeInterrupt(heart, stuff, FALLING);
  Serial.begin (57600); // 9600 bps
  queue.setPrinter(Serial);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10);
}

void stuff() 
{
   unsigned long average = 0;
   if(first) {
     start = micros();
     first = false;
     return;
   }
   else {
     ended = micros();
     diff = ended - start; 
     start = micros();
   }
   
   if(queue.count() != numBeats) {
      queue.push(diff);
      return;
   }
   else {
     queue.pop();
     queue.push(diff);
     average = getAverage();
   }
   
   unsigned long time = 60*1000000 / average;
   Serial.println(time); 
  
}

unsigned long getAverage() {
  unsigned long avg = 0;
  for(int i = 0; i < numBeats; i++) {
    unsigned long temp = queue.pop();
    avg += temp;
    queue.push(temp);    
  }
 return avg/10; 
}
