
// SETTINGS ///////////////////
#define sensorPin A0
#define N 100
// sampleRate in Hz
#define sampleRate 20
// You should think about the sampleRate and N carefully.
// For example if the sampleRate is 20 and N is 100, then we are storing the previous
// 100 / 20 = 5 seconds worth of data. This is the time scale over which we will compute
// the average and standard deviation, which are used for spike detection.

// where threads are plugged in
int threadPin[6] = {3, 5, 6, 7, 8, 9};
// how much power you think each thread needs. depends on their length etc
int threadPower[6] = {255, 255, 255, 255, 255, 255};
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

long threadTimeOn[6] = {0, 0, 0, 0, 0, 0};
bool threadOn[6] = {false, false, false, false, false, false};
int whichThread = 0;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 6; i++) {
    pinMode(threadPin[i], OUTPUT);
  }
}

void loop() {
  sensorRaw = analogRead(sensorPin);
  sensorFilteredNew = myFilter.step(sensorRaw);
  addToArray(sensorFilteredNew);
  
  if (hasPeak()) {
    Serial.print("MARK");Serial.print(",");
    threadOn[whichThread] = true;
    threadTimeOn[whichThread] = millis();
    whichThread += 1;
    whichThread %= 6;
  }
  Serial.print(sensorFilteredNew);Serial.print(",");
  //Serial.print(sensorRaw);Serial.print(",");
  
  updateThreads();

  delay(samplePeriod);
}

int threadLastPowered = 0; // this should only be used by updateThreads()
void updateThreads() {
  // only turn on one thread each time, then wait for the next time to come around to turn on
  // a different thread. should keep track of which thread was turned on last time and turn on
  // a different thread this time
  
  analogWrite(threadPin[threadLastPowered], 0);
  
  int startThread = (threadLastPowered + 1) % 6;
  for (int j = 0; j < 6; j++) {
    int i = (startThread + j + 1) % 6;
    if (threadOn[i]) {
      Serial.print("\nthreadPin: ");Serial.print(threadPin[i]);Serial.println();
      analogWrite(threadPin[i], threadPower[i]);
      threadLastPowered = i;
      break;
    } else {
      analogWrite(threadPin[i], 0);
    }
  }
  
  for (int i = 0; i < 6; i++) {
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
  //Serial.print("\n avg: ");Serial.print(avg);

  // calculate the standard deviation
  float std = 0;
  for (int i = 0; i < N; i++) {
    std += pow(sensorFiltered[i] - avg, 2);
  }
  std = sqrt(std / N);
  //Serial.print(" std: ");Serial.println(std);
  
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






