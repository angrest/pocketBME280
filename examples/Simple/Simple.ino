#include <pocketBME280.h>

pocketBME280 mySensor;

void setup() {
  Serial.begin(115200);

  Serial.println(F("Initializing BME280 Sensor"));
  if (mySensor.begin() != 0x60) {
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
  Serial.print(mySensor.getTemperature() / 100.);
  Serial.println(F("°C"));
  Serial.print(F("Pressure: "));
  Serial.print(mySensor.getPressure());
  Serial.println(F(" Pa"));
  Serial.print(F("Humidity: "));
  Serial.print(mySensor.getHumidity() / 1024.);
  Serial.println(F("%rH"));

  delay(5000);
}
