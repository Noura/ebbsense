
// SETTINGS ///////////////////
#define sensorPin A0
#define N 32
// sampleRate in Hz
#define sampleRate 20
// if the sensor value increases by this much in the array of previous values
// we store, then it counts as a "peak"
int peakThreshold = 50;
///////////////////////////////

int sensorRaw;
int sensorFilteredNew;
int sensorFiltered[N];
int sensorIncrease;

int samplePeriod = 1000 / sampleRate;

//Low pass bessel filter order=1 alpha1=0.02 
/* designed at http://www.schwietering.com/jayduino/filtuino/
 * http://www.schwietering.com/jayduino/filtuino/index.php?characteristic=be&passmode=lp&order=1&usesr=usesr&sr=20&frequencyLow=0.4&noteLow=&noteHigh=&pw=pw&calctype=float&run=Send
 */
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

void setup() {
  Serial.begin(9600);
}

void loop() {
  sensorRaw = analogRead(sensorPin);
  sensorFilteredNew = myFilter.step(sensorRaw);
  addToArray(sensorFilteredNew);
  sensorIncrease = maxIncrease();
  
  if (sensorIncrease >= peakThreshold) {
    Serial.print("MARK");
  } else {
    Serial.print(sensorFilteredNew);
  }
  Serial.print(",");
  
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
}




