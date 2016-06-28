/* Wearable Color-Changing Shirt for Summer 2016
   2016 Noura Howell
   
   This is intended to go with a shirt with a few color-changing threads. The
   wearer of the shirt also wears a Bitalino EDA sensor. All this is connected
   to our PCB which has an Arduino Mini running this sketch.
   
   When the wearer's EDA spikes, this "moment" is displayed by making the
   threads change color. First one thread changes color, then as it fades the
   next thread changes, and so on - a sort of "ripple effect" for each moment.
*/

#include <EDA.h>
#include <PrintArray.h>
#include <Threads.h>

/////////////////////////////////////////////////////////////////
// SETTINGS /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// how many threads we are using
#define N_THREADS 3
// how long to display each moment for
#define MOMENT_DT (3 * 60 * 1000)

// the pin the EDA sensor is connected to
#define edaSensorPin A7

// TODO the pin the FSR sensor is connected to
// TODO pressing this FSR forces a "moment"
#define fsrMomentOnPin A0
// the pin the FSR sensor is connected to
// TODO touching this FSR turns off the threads
#define fsrMomentOffPin A1
// readings greater than this means FSR is being touched
#define fsrThreshold 800

// all pins that the PCB has for thread outputs - some may be broken on this PCB
int allThreadPins[] = {5, 4, 3, 2};
// where threads are actually plugged in - these pins are working
int threadPins[] = {5, 4, 3};
// how much power each thread needs. depends on their length etc
int threadPowerLevels[] = {255, 255, 255};
// how long (ms) each thread should get power for to change color
unsigned long threadPowerDurations[] = { (1 * 60 * 1000), (1 * 60 * 1000), (1 * 60 * 1000)};
///////////////////////////////

// EDA sensor & spike detection
EDA myEDASensor;

// threads controller
Threads myThreads;
// specify which threads we want to be on at any given time
bool threadsOn[N_THREADS];

// log when moments happen
unsigned long momentTimes[N_THREADS];

// temp variables for repeated use in loop()
int edaVal;
unsigned long t;
unsigned long dt;
int thread_index;
bool momentWasLogged;

/* TEST: working up to MAIN but using the Threads library and moments */

void setup() {
  Serial.begin(9600);
  
  for (int i = 0; i < N_THREADS; i++) {
    // initialize all momentTimes to 0
    momentTimes[i] = 0;
    // make sure all threads are off
    threadsOn[i] = false;
    analogWrite(threadPins[i], 0);
  }
  
  myThreads.init(N_THREADS, threadPins, threadPowerLevels, threadPowerDurations);
}

void loop() {
  Serial.println();
  
  t = millis();
  
  edaVal = analogRead(edaSensorPin);
  myEDASensor.update(edaVal);
  
  if ( myEDASensor.hasPeak() ) {
    Serial.println("EDA SENSOR PEAK");
    momentWasLogged = logMoment(t);
    Serial.print("momentWasLogged: ");Serial.println(momentWasLogged);
  }
  
  if (analogRead(fsrMomentOnPin) > fsrThreshold) {
    Serial.println("FSR MOMENT ON PRESS");
    momentWasLogged = logMoment(t);
    Serial.print("momentWasLogged: ");Serial.println(momentWasLogged);
  }
  
  for (int i = 0; i < N_THREADS; i++) {
    threadsOn[i] = false;
  }
  
  for (int i = 0; i < N_THREADS; i++) {
    if (momentTimes[i] == 0) continue;
    dt = abs(t - momentTimes[i]);
    thread_index = dt / MOMENT_DT;
    if (thread_index < N_THREADS) {
      threadsOn[thread_index] = true;
    }
  }
  
  myThreads.update(threadsOn);
  
  Serial.print("momentTimes: ");
  PrintArray::printArrayUnsignedLong(momentTimes, N_THREADS);
  Serial.print("threadsOn: ");
  PrintArray::printArrayBool(threadsOn, N_THREADS);
}

bool logMoment(unsigned long t) {
  // if we already logged the moment, don't log it again
  if ( momentTimes[0] != 0 && abs(t - momentTimes[0]) < MOMENT_DT ) {
    return false;
  }
  
  // OK we know it's a distinct moment that we should record in our FIFO array
  for (int i = N_THREADS - 1; i >= 1; i--) {
    momentTimes[i] = momentTimes[i-1];
  }
  momentTimes[0] = t;
  return true;
}

/* TEST: all 4 debug LEDs on the PCB 
   NOTE: make sure the debug switch is on so the LEDs can light up

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    analogWrite(allThreadPins[i], 0);
  }
}

void loop() {
  for (int i = 0; i < 4; i++) {
    Serial.print("\nturning on pin ");Serial.println(allThreadPins[i]);
    analogWrite(allThreadPins[i], 255);
    delay(1000);
    Serial.println("turning that pin off");
    analogWrite(allThreadPins[i], 0);
  }
}*/

/* OLD MAIN: for turning one thread on and off
void setup() {
  Serial.begin(9600);
  pinMode(threadPin, OUTPUT);
  threadTimeOn = 0;
  threadOn = false;
  pinMode(buttonPin, INPUT);
  digitalWrite(threadPin, 0);
}

void loop() {
  int sensorRaw = myEDA.update();
  
  // the button lets the user force the thread to turn on
  int buttonVal = digitalRead(buttonPin);
  
  // TODO fsrOff lets the user force the threads to turn off
  fsrOffVal = analogRead(fsrOffPin);
  
  if (hasPeak() || buttonVal > 0) {
    // sending data to Processing sketch which graphs it
    Serial.print("MARK");Serial.print(",");
    if (!threadOn) {
      analogWrite(threadPin, 255);
      threadTimeOn = millis();
      threadOn = true;
    }
  } else {
    if (threadOn && millis() - threadTimeOn > threadStayOnFor) {
      analogWrite(threadPin, 0);
      threadOn = false;
    }
  }
  
  // sending data to Processing sketch which graphs it
  Serial.print(sensorRaw);Serial.print(",");
}*/

/* TEST: reading fsrOff
void setup () {}

void loop() {
  fsrOffVal = analogRead(fsrOffPin);
  Serial.println(fsrOffVal);
  if (fsrOffVal > fsrOffThreshold) {
    Serial.println("TOUCH"); 
  }
  delay(100);
}*/

/* TEST: powering the threads
void setup () {}

void loop() {
  analogWrite(5, 255);
}*/

/* TEST: EDA.h
void setup() {
  Serial.begin(9600);
}

void loop() {
  int edaVal = analogRead(edaSensorPin);
  myEDA.update(edaVal);
  Serial.print(edaVal);Serial.print(",");
  if ( myEDA.hasPeak() ) {
    Serial.print("MARK");Serial.print(","); 
  }
}*/

/* TEST: reading edaSensorPin directly
void setup() {
  Serial.begin(9600); 
}

void loop() {
  int edaVal = analogRead(edaSensorPin);
  Serial.print(edaVal);Serial.print(",");
}*/





