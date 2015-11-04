
// SETTINGS ///////////////////
#define sensorPin A0
#define N 100
// sampleRate in Hz
#define sampleRate 20
// You should think about the sampleRate and N carefully.
// For example if the sampleRate is 20 and N is 100, then we are storing the previous
// 100 / 20 = 5 seconds worth of data. This is the time scale over which we will compute
// the average and standard deviation, which are used for spike detection.

#define NPINS 1
// where threads are plugged in
int threadPin[NPINS] = {3};
// how much power you think each thread needs. depends on their length etc
int threadPower[NPINS] = {255};
// how long (ms) a thread should stay on for before turning off
#define threadStayOnFor 10000
///////////////////////////////

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

long threadTimeOn[NPINS];
bool threadOn[NPINS];

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < NPINS; i++) {
    pinMode(threadPin[i], OUTPUT);
    threadTimeOn[i] = 0;
    threadOn[i] = false;
  }
}

void loop() {
  sensorRaw = analogRead(sensorPin);
  sensorFilteredNew = myFilter.step(sensorRaw);
  addToArray(sensorFilteredNew);
  
  if (hasPeak()) {
    Serial.print("MARK");Serial.print(",");
    activateThread();
  }
  Serial.print(sensorFilteredNew);Serial.print(",");
  //Serial.print(sensorRaw);Serial.print(",");
  
  updateThreads();

  delay(samplePeriod);
}

// which thread to activate this time. should only be used by activateThread()
int whichThread = 0;
void activateThread() {
  threadOn[whichThread] = true;
  threadTimeOn[whichThread] = millis();
  
  // choosing which thread to activate next
  // this cycles through all the available threads
  whichThread += 1;
  whichThread %= NPINS;
}

// which thread last received power. should only be used by activateThreads()
int threadLastPowered = 0;
void updateThreads() {
  // only turn on one thread each time, then wait for the next time to come around to turn on
  // a different thread. should keep track of which thread was turned on last time and turn on
  // a different thread this time
  
  analogWrite(threadPin[threadLastPowered], 0);
  
  int startThread = (threadLastPowered + 1) % NPINS;
  for (int j = 0; j < NPINS; j++) {
    int i = (startThread + j) % NPINS;
    if (threadOn[i]) {
      analogWrite(threadPin[i], threadPower[i]);
      threadLastPowered = i;
      break;
    } else {
      analogWrite(threadPin[i], 0);
    }
  }
  
  for (int i = 0; i < NPINS; i++) {
    if (threadOn[i] && millis() - threadTimeOn[i] > threadStayOnFor) {
      threadOn[i] = false;
    }
  }
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
  // then that counts as a peak
  if (sensorFiltered[0] - avg > std &&
      sensorFiltered[0] - avg > 25 ) {
    return true;
  } else {
    return false;
  }
}

/*
int maxIncrease() {
  int maxVal = sensorFiltered[0];
  int maxIncr = 0;
  for (int i = 0; i < N; i++) {
    int curIncr = maxVal - sensorFiltered[i];
    if (curIncr > maxIncr) {
      maxIncr = curIncr;
    }
    maxVal = max(maxVal, sensorFiltered[i]);
  }
  return maxIncr;
}*/






