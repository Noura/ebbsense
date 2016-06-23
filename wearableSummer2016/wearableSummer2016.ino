#include <EDA.h>

/////////////////////////////////////////////////////////////////
// SETTINGS /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// the pin the EDA sensor is plugged into
#define edaSensorPin A7

#define buttonPin 10

// TODO touching this FSR turns off the threads
#define fsrOffPin A0
// readings greater than this means FSR is being touched
#define fsrOffThreshold 800

// where threads are plugged in
int threadPin = 5;
// how much power you think each thread needs. depends on their length etc
int threadPower = 255;
// how long (ms) a thread should stay on for before turning off
#define threadStayOnFor 180000 //180000
///////////////////////////////

EDA myEDA;

int fsrOffVal = 0;

// the time at which a thread was activated.
// threads get deactivated after threadStayOnFor milliseconds
long threadTimeOn;
// whether each thread is currently activated or not
bool threadOn;

/* MAIN: for actually doing what we want to do */
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
}

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





