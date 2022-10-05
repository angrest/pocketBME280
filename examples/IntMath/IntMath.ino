//****************************************************************************//
//
//  pocketBME280 Library
//  Axel Grewe - angrest@gmx.net 12-Sep-2022
//
//  Example using integer arithmetics and own wire object and setting the
//  I2C address of the sensor.
//
//****************************************************************************//

#include <pocketBME280.h>

pocketBME280 mySensor;

// This is not necessary here, just to show the usage
#if defined(ESP32)
TwoWire myWire = TwoWire(0);
#elif defined(ESP8266)
TwoWire myWire = TwoWire();
#else
TwoWire myWire;
#endif

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing BME280 Sensor");

  mySensor.setAddress(0x76); // not necessary, just to show usage

  if (!mySensor.begin(myWire)) {
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
  int32_t temp = mySensor.getTemperature();
  Serial.print(temp / 100);
  Serial.print(".");
  Serial.print(temp % 100);
  Serial.println("°C");
  Serial.print("Pressure: ");
  Serial.print(mySensor.getPressure());
  Serial.println(" Pa");
  Serial.print("Humidity: ");
  int32_t hum = mySensor.getHumidity();
  Serial.print( hum / 1024);
  Serial.print(".");
  Serial.print( ((hum % 1024) * 100)  / 1024);
  Serial.println("% rH");

  delay(5000);
}
