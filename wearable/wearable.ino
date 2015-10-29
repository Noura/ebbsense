#include <firFilter.h>

#define sensorPin A0

firFilter Filter;

int sensorRaw;
int sensorFiltered;

void setup() {
  Serial.begin(9600);
  Filter.begin();
}

void loop() {
  sensorRaw = analogRead(sensorPin);
  sensorFiltered = Filter.run(sensorRaw);
  Serial.print(sensorFiltered);Serial.print(",");
}
