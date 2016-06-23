
// SETTINGS ///////////////////
// the pin the GSR sensor is plugged into
#define sensorPin A7
// the number of previous filtered GSR readings to store
#define N 100
// sample rate of taking GSR readings in Hz
#define sampleRate 20
// You should think about the sampleRate and N carefully.
// For example if the sampleRate is 20 and N is 100, then we are storing the previous
// 100 / 20 = 5 seconds worth of data. This is the time scale over which we will compute
// the average and standard deviation, which are used for spike detection.

#define buttonPin 10

// where threads are plugged in
int threadPin = 4;
// for each thread, have an LED to show when the thread is activated
// (the LED only lights up when in debug mode)
int ledPin = 13;
// how much power you think each thread needs. depends on their length etc
int threadPower = 255;
// how long (ms) a thread should stay on for before turning off
#define threadStayOnFor 1000 //180000
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
long threadTimeOn;
// whether each thread is currently activated or not
bool threadOn;

// whether we are in debug mode or not. in debug mode,
// LEDs come on corresponding with which threads are on
bool debug = true;

// the timestamp when the sketch started
long startTime;

void setup() {
  Serial.begin(9600);
  pinMode(threadPin, OUTPUT);
  threadTimeOn = 0;
  threadOn = false;
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(threadPin, 0);
  startTime = millis();
}

void loop() {
  sensorRaw = analogRead(sensorPin);
  sensorRaw = min(sensorRaw, 1023);
  sensorRaw = max(sensorRaw, 0);
  sensorFilteredNew = myFilter.step(sensorRaw);
  addToArray(sensorFilteredNew);
  
  // the button lets the user force the thread to turn on
  int buttonVal = digitalRead(buttonPin);
  
  if (hasPeak() || buttonVal > 0) {
    // sending data to Processing sketch which graphs it
    Serial.print("MARK");Serial.print(",");
    if (!threadOn) {
      // turn on a thread to show this peak
      //Serial.println("\nTURNING THREAD ON");
      analogWrite(threadPin, 255);
      digitalWrite(ledPin, HIGH);
      threadTimeOn = millis();
      threadOn = true;
    }
  } else {
    if (threadOn && millis() - threadTimeOn > threadStayOnFor) {
      //Serial.println("\nTURNING THREAD OFF");
      analogWrite(threadPin, 0);
      digitalWrite(ledPin, LOW);
      threadOn = false;
    }
  }
  // sending data to Processing sketch which graphs it
  //Serial.print(sensorFilteredNew);Serial.print(",");
  Serial.print(sensorRaw);Serial.print(",");

  // wait this much time so that we regularly sample
  // the GSR sensor
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
  // ignore initial peak due to initialization
  if ( millis() - startTime < 1000 ) {
    return false;
  }
  
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
  if (sensorFiltered[0] - avg > std
       &&
       sensorFiltered[0] - avg > 50 ) {
    return true;
  } else {
    return false;
  }
}







