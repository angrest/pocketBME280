#include <pocketBME280.h>

pocketBME280 mySensor;
TwoWire myWire;   // not necessary, just to show usage


void setup() {
  Serial.begin(115200);

  Serial.println(F("Initializing BME280 Sensor"));

   mySensor.setAddress(0x76); // not necessary, just to show usage
  
  if (mySensor.begin(myWire) != 0x60) {
    Serial.println(F("No BME280 sensor. Check wiring."));
    while (1)
      ;  //Freeze
  }
}

void loop() {
  mySensor.startMeasurement();
  while (!mySensor.isMeasuring()) {
    Serial.println(F("Waiting for Measurement to start"));
    delay(1);
  }
  while (mySensor.isMeasuring()) {
    Serial.println(F("Measurement in progress"));
    delay(1);
  }
  Serial.print(F("Temperature: "));
  int32_t temp = mySensor.getTemperature();
  Serial.print(temp / 100);
  Serial.print(F("."));
  Serial.print(temp % 100);
  Serial.println(F("°C"));
  Serial.print(F("Pressure: "));
  Serial.print(mySensor.getPressure());
  Serial.println(F(" Pa"));
  Serial.print(F("Humidity: "));
  int32_t hum = mySensor.getHumidity();
  Serial.print( hum / 1024);
  Serial.print(F("."));
  Serial.print( ((hum % 1024) * 100)  / 1024);
  Serial.println(F("% rH"));

  delay(5000);
}
