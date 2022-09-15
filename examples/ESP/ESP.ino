//****************************************************************************//
//
//  pocketBME280 Library
//  Axel Grewe - angrest@gmx.net 12-Sep-2022
//
//  Example showing use of different ports for I2C connection on ESP boards
//
//****************************************************************************//

#include <pocketBME280.h>

pocketBME280 mySensor;

// set up alternative Port
#if defined(ESP32)
TwoWire myWire = TwoWire(0);
#define I2C_SDA 26
#define I2C_SCL 25
#elif defined(ESP8266)
TwoWire myWire = TwoWire();
#define I2C_SDA 4
#define I2C_SCL 5
#else
TwoWire myWire;
#endif

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing BME280 Sensor");
#if defined(ESP8266)||defined(ESP32)
  myWire.begin(I2C_SDA, I2C_SCL);
#endif

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
