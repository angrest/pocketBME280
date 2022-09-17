# pocketBME280

**Attention: unfortunately, the Library Manager proposes due to a mistake version 2.3.1 as latest version. This is, however, a pre-release version. Please use version 1.2.1 and ignore update requests to 2.3.1 from the library manager for now.**

## Description
This library provides a compact option to read out data from a BME280 sensor via I2C. It is optimised to be used for weather monitoring in Arduino projects. To be most compatible, only 32bit integer arithmetics is used internally. Floating point values can be easily derived in the Arduino sketch.
The sensor settings and compensation formula follow the recommendations from Bosch given in the [BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/) [datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf).

## Basic function
The library implements an on-demand measurement using the "forced mode". This means:

- by default the sensor is in sleep mode using very little power
- if needed a measurement cycle is started
- after about 10ms, the measurement is finished and the sensor goes back in sleep mode automatically
- until next the next measurement is finished, the data can be read

To calculate temperature, pressure and humidity from the raw sensor data the 32 bit algorithm given by Bosch in the data sheet is implemented. To save energy, no oversampling ("single") is used.

## Programming
The library provides the functions
1. `begin()` initialize the communication and sensor. Returns `true` if a BME280 or BMP280 sensor is found. Optionally, you can provide your own `TwoWire` instance to be used: `begin(TwoWire &wirePort)`. The latter makes most sense if you do not use standard I2C ports on the ESP-Plattform. 
2. `setAddress(unit8_t address)` optionally set the I2C address of the sensor (default is `0x76`, you might try `setAddress(0x77)` if the sensor does not respond
3. `reset()`reset sensor. Need to call `begin()` afterwards
4. `startMeasurement()` kick off a measurement cycle. The sensor goes back to sleep automatically thereafter.
5. `bool isMeasuring()` returns `true` while a measurement is being performed.
6. `uint32_t getTemperature()` returns the temperature in units of 0.01 DegC. An output value of "5123" equals to 51.23 DegC
7. `uint32_t getPressure()`returns the pressure in Pa. An output value of "96386" equals to 96386 Pa (or 963.86 hPa)
8. `uint32_t getHumidity()` returns the humidity in Q22.10 format (22 integer and 10 fractional bits). An output value of “47445” equals to 47445/1024 = 46. 333 %RH
The values for temperature, pressure and humidity are only read from the sensor once per measurement. Thus, subsequent calls to `getTemperature()` or the equivalents will return the same value until a new measurement is started. As for pressure and humidity calculations the temperature value is needed, `getTemperature()` should be called first (however, the library internally keeps track and sorts that out should you call the `get*()`-functions in a different order).

A typical measurement in the `loop()`would look like this (the complete sketch is in the [example folder](examples/Simple/Simple.ino)):
```
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
  Serial.print("Humidity: "));
  Serial.print(mySensor.getHumidity() / 1024.);
  Serial.println("%rH");

  delay(5000);
}
```

**Important:** 
- The sensor will actually need a few millis before `isMeasuring()` returns `true`. In the above example, the meassage is printed once on an Arduino Pro Mini. On other boards, this may differ.
- If you call the `get*()`-functions before the measurement is finished, the sensor will return the data from the last previous measurement which was finished. This may or may not be important, depending on the time between subsequent measurements.
- `get*()` will return the same values until the next time `startMeasurement()` is called (and the measurement is finished!).
