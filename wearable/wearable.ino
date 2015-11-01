
// SETTINGS ///////////////////
#define sensorPin A0
#define N 100
// sampleRate in Hz
#define sampleRate 20
// You should think about the sampleRate and N carefully.
// For example if the sampleRate is 20 and N is 100, then we are storing the previous
// 100 / 20 = 5 seconds worth of data. This is the time scale over which we will compute
// the average and standard deviation, which are used for spike detection.
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

void setup() {
  Serial.begin(9600);
}

void loop() {
  sensorRaw = analogRead(sensorPin);
  sensorFilteredNew = myFilter.step(sensorRaw);
  addToArray(sensorFilteredNew);
  
  //Serial.print(s);Serial.print(" ");Serial.print(sensorIncrease);Serial.println();
  
  if (hasPeak()) {
    Serial.print("MARK");Serial.print(",");
  }
  Serial.print(sensorFilteredNew);Serial.print(",");
  
  delay(samplePeriod);
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
  
  if (sensorFiltered[0] - avg > std) {
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






