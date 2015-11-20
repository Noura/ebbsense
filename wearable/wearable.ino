
// SETTINGS ///////////////////
// the pin the GSR sensor is plugged into
#define sensorPin A7
// the pin the debug switch is plugged into
#define debugPin 13
// the number of previous filtered GSR readings to store
#define N 100
// sample rate of taking GSR readings in Hz
#define sampleRate 20
// You should think about the sampleRate and N carefully.
// For example if the sampleRate is 20 and N is 100, then we are storing the previous
// 100 / 20 = 5 seconds worth of data. This is the time scale over which we will compute
// the average and standard deviation, which are used for spike detection.

// the number of threads on the wearable
#define NTHREADS 1
// where threads are plugged in
int threadPin[NTHREADS] = {5};
// for each thread, have an LED to show when the thread is activated
// (the LED only lights up when in debug mode)
int ledPin[NTHREADS] = {13};
// how much power you think each thread needs. depends on their length etc
int threadPower[NTHREADS] = {120};
// how long (ms) a thread should stay on for before turning off
#define threadStayOnFor 12000
///////////////////////////////

// NOTE
// throughout, "activated" or "on" for threads is used interchangeably
// and "deactivated" or "off" for threads is used interchangeably
// but "supplying power" to a thread is different, because all the
// threads that are "on" take turns getting supplied power in order
// to accommodate the amount of current the battery can supply.

// Low pass bessel filter order=1 alpha1=0.02 samplerate=20
// designed at http://www.schwietering.com/jayduino/filtuino/
// http://www.schwietering.com/jayduino/filtuino/index.php?characteristic=be&passmode=lp&order=1&usesr=usesr&sr=20&frequencyLow=0.4&noteLow=&noteHigh=&pw=pw&calctype=float&run=Send
class filter
{
  public:
    filter()
    {
      v[0]=0.0;
    }
  private:
    float v[2];
  public:
    float step(float x) //class II 
    {
      v[0] = v[1];
      v[1] = (5.919070381841e-2 * x)
             + (  0.8816185924 * v[0]);
      return (v[0] + v[1]);
    }
};

filter myFilter;

int sensorRaw;
int sensorFilteredNew;
int sensorFiltered[N];

int samplePeriod = 1000 / sampleRate;

// the time at which a thread was activated.
// threads get deactivated after threadStayOnFor milliseconds
long threadTimeOn[NTHREADS];
// whether each thread is currently activated or not
bool threadOn[NTHREADS];

// whether we are in debug mode or not. in debug mode,
// LEDs come on corresponding with which threads are on
bool debug = false;

void setup() {
  Serial.begin(9600);
  pinMode(debugPin, INPUT);
  for (int i = 0; i < NTHREADS; i++) {
    pinMode(threadPin[i], OUTPUT);
    threadTimeOn[i] = 0;
    threadOn[i] = false;
//    pinMode(ledPin[i], OUTPUT);
  }
}

void loop() {
  //debug = (digitalRead(debugPin) == HIGH);
  debug = true;
  
  sensorRaw = analogRead(sensorPin);
  sensorRaw = min(sensorRaw, 1023);
  sensorRaw = max(sensorRaw, 0);
  sensorFilteredNew = myFilter.step(sensorRaw);
  addToArray(sensorFilteredNew);
  
  if (hasPeak()) {
    // sending data to Processing sketch which graphs it
    Serial.print("MARK");Serial.print(",");
    // turn on a thread to show this peak
    activateThread();
  }
  // sending data to Processing sketch which graphs it
  Serial.print(sensorFilteredNew);Serial.print(",");
  
  // keeps track of giving power to the threads that are
  // currently on, as well as turning threads off after
  // stayOnFor milliseconds since the thread came on
  updateThreads();

  // wait this much time so that we regularly sample
  // the GSR sensor
  delay(samplePeriod);
}

// which thread to activate this time. should only be used by activateThread()
int whichThread = 0;
void activateThread() {
  // you have to do these three things to turn a thread "on"
  threadOn[whichThread] = true;
  threadTimeOn[whichThread] = millis();
  if (debug) {
    //Serial.println();Serial.println("turning on LED");
    digitalWrite(ledPin[whichThread], HIGH);
  }
  
  // choosing which thread to activate next
  // this cycles through all the available threads
  whichThread += 1;
  whichThread %= NTHREADS;
}

// which thread last received power. should only be used by activateThreads()
int threadLastPowered = 0;
void updateThreads() {
  // 1. makes all the activated threads take turns getting power from
  //    the battery. each time updateThreads() is called, it chooses
  //    a different activated thread to receive power.
  // 2. turns off threads after stayOnFor ms since thread came on.
  // 3. makes sure to turn off debug LEDs
  
  // the last thread to get power already got some power, so its turn
  // is over. stop giving it power.
  analogWrite(threadPin[threadLastPowered], 0);
  
  // choose the next thread to give power to. start by looking at the
  // the next thread right after threadLastPowered, and then keep
  // checking each thread one by one until we find one that is on.
  // once we find one that is on, we can give that thread power and
  // stop looking/break out of the for loop
  int startThread = (threadLastPowered + 1) % NTHREADS;
  for (int j = 0; j < NTHREADS; j++) {
    int i = (startThread + j) % NTHREADS;
    if (threadOn[i]) {
      //Serial.println();
      //Serial.print("powering thread at index: ");Serial.println(i);
      analogWrite(threadPin[i], threadPower[i]);
      threadLastPowered = i;
      break;
    } else {
      // also just make sure any "off" pins are not getting power.
      // not sure if this is necessary here?
      analogWrite(threadPin[i], 0);
    }
  }
  
  // go through all the threads and see which have been on for long
  // enough that it is time to turn them off. also turn off the
  // corresponding LED for that thread.
  for (int i = 0; i < NTHREADS; i++) {
    if (threadOn[i] && millis() - threadTimeOn[i] > threadStayOnFor) {
      threadOn[i] = false;
      digitalWrite(ledPin[i], LOW);
    }
  }
  
  // if we are not in debug mode, then all LEDs should be off.
//  if (!debug) {
//    for (int i = 0; i < NTHREADS; i++) {
//      digitalWrite(ledPin[i], LOW);
//    }
//  }
}

void addToArray(int x) {
  // index 0 holds the most recent value
  // index N-1 holds the oldest value
  for (int i = N - 1; i >= 1; i--) {
    sensorFiltered[i] = sensorFiltered[i-1];
  }
  sensorFiltered[0] = x;
}

bool hasPeak() {
  // calculate the average
  float avg = 0;
  for (int i = 0; i < N; i++) {
    avg += sensorFiltered[i];
  }
  avg /= N;

  // calculate the standard deviation
  float std = 0;
  for (int i = 0; i < N; i++) {
    std += pow(sensorFiltered[i] - avg, 2);
  }
  std = sqrt(std / N);
  
  // if the current value is more than a standard deviation above the mean,
  // then that counts as a peak. also make sure that the current value is
  // more than 25 above the mean, in case there is just some noise in a
  // pretty flat signal
  if (sensorFiltered[0] - avg > std &&
      sensorFiltered[0] - avg > 50 ) {
    return true;
  } else {
    return false;
  }
}






