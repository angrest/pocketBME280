//****************************************************************************//
//
//  pocketBME280 Library
//  Axel Grewe - angrest@gmx.net 12-Sep-2022
//
//  Example for basic usage of library
//
//****************************************************************************//
#include <pocketBME280.h>

pocketBME280 mySensor;

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing BME280 Sensor");
  if (!mySensor.begin()) {
    Serial.println("No BME280 sensor. Check wiring.");
    while (1)
      ;  //Freeze
  }
}

void loop() {
  mySensor.startMeasurement();
  while (!mySensor.isMeasuring()) {
    Serial.println("Waiting for Measurement to start");
    delay(1);
  }
  while (mySensor.isMeasuring()) {
    Serial.println("Measurement in progress");
    delay(1);
  }
  Serial.print("Temperature: ");
  Serial.print(mySensor.getTemperature() / 100.);
  Serial.println("°C");
  Serial.print("Pressure: ");
  Serial.print(mySensor.getPressure());
  Serial.println(" Pa");
  Serial.print("Humidity: ");
  Serial.print(mySensor.getHumidity() / 1024.);
  Serial.println("%rH");

  delay(5000);
}
