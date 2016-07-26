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
// FOR LIGHT BROWN SHIRT ////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// how many threads we are using
#define N_THREADS 3
// how long to display each moment for
// MOMENT_DT - threadPowerDurations[i] is how long thread_i has to cool off
#define MOMENT_DT 150000

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
// TODO I think pin 2 is not even a PWM out
int allThreadPins[] = {5, 6, 3, 2};

// where threads are actually plugged in - these pins are working
int threadPins[] = {5, 6, 3};

// change pretty quickly, maybe don't leave on too long at this power level?
// third thread doesn't change as much in some environments.
// how much power each thread needs. depends on their length etc
// int threadPowerLevels[] = {30, 30, 30};
// how long (ms) each thread should get power for to change color
// unsigned long threadPowerDurations[] = { 10000, 10000, 10000 };

// change pretty quickly, maybe don't leave on too long at this power level?
// third thread needs a bit more power so here it's good
// int threadPowerLevels[] = {30, 30, 40};
// unsigned long threadPowerDurations[] = { 20000, 20000, 20000 };

// change very slowly, seems OK to leave it on for a pretty long while
// although it does get warm
//int threadPowerLevels[] = {15, 18, 20};
//unsigned long threadPowerDurations[] = { 120000, 120000, 120000 };

int threadPowerLevels[] = {35, 35, 35};
unsigned long threadPowerDurations[] = { 120000, 120000, 120000 };
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
  
  // make sure ALL thread pins are off, even the ones we aren't using
  for (int i = 0; i < 4; i++) {
    analogWrite(allThreadPins[i], 0);
  }
  
  for (int i = 0; i < N_THREADS; i++) {
    // initialize all momentTimes to 0
    momentTimes[i] = 0;
    // initialize it so all threads should be off
    threadsOn[i] = false;
  }
  
  myThreads.init(N_THREADS, threadPins, threadPowerLevels, threadPowerDurations);
}

void loop() {
  Serial.println();
  
  t = millis();
  Serial.print("t: ");Serial.println(t);
  
  Serial.print("fsrON: ");Serial.println(analogRead(fsrMomentOnPin));
  Serial.print("fsrOFF: ");Serial.println(analogRead(fsrMomentOffPin));
  
  // if the EDA sensor has a peak, log that as a moment
  edaVal = analogRead(edaSensorPin);
  Serial.print("edaVal: ");Serial.println(edaVal);
  myEDASensor.update(edaVal);
  if ( myEDASensor.hasPeak() ) {
    Serial.println("EDA SENSOR PEAK");
    momentWasLogged = logMoment(t);
    Serial.print("momentWasLogged: ");Serial.println(momentWasLogged);
  }
  
  // if the fsrMomentOn is pressed, log that as a moment
  if (analogRead(fsrMomentOnPin) > fsrThreshold) {
    Serial.println("FSR MOMENT ON PRESS");
    momentWasLogged = logMoment(t);
    Serial.print("momentWasLogged: ");Serial.println(momentWasLogged);
  }
  
  // if the fsrMomentOff is pressed, remove all moments
  if (analogRead(fsrMomentOffPin) > fsrThreshold) {
    Serial.println("FSR MOMENT OFF PRESS");
    for (int i = 0; i < N_THREADS; i++) {
      momentTimes[i] = 0;
    }
  }
  
  // deciding which threads should be on
  for (int i = 0; i < N_THREADS; i++) {
    // refresh all threads to be off for this loop
    threadsOn[i] = false;
  }
  for (int i = 0; i < N_THREADS; i++) {
    // check each moment time.
    // if there wasn't a moment, do nothing
    if (momentTimes[i] == 0) continue;
    // how long ago the moment happened determines which thread should be on to
    // display that moment
    dt = abs(t - momentTimes[i]);
    thread_index = dt / MOMENT_DT;
    if (thread_index < N_THREADS) {
      threadsOn[thread_index] = true;
    }
  }
  // threads controller will make sure the threads we specify are on
  myThreads.update(threadsOn);
  
  Serial.print("momentTimes: ");
  PrintArray::printArrayUnsignedLong(momentTimes, N_THREADS);
  Serial.print("threadsOn: ");
  PrintArray::printArrayBool(threadsOn, N_THREADS);
  
  delay(500);
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





