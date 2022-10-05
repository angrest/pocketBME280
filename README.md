# pocketBME280

## Description
This library provides a compact option to read out data from a BME280 sensor via I2C. It is designed to be used for weather monitoring in Arduino projects. To optimize size and compatibility, internal arithmetics is based entirely on 32bit integers. If floating point values are required, they can easily be calculated as shown in the [IntMath example](examples/IntMath/IntMath.ino).
The sensor settings and compensation formulae follow the recommendations from Bosch given in the [BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/) [datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf) section 3.5.1 for the settings and 4.2.3 as well as 8.2 for the compensation algorithms. 

## Basic function
The library implements an on-demand measurement using the "forced mode" (section 3.3 in the datasheet). This means:

- by default the sensor is in sleep mode using very little power
- if needed a measurement cycle is started
- after about 10ms, the measurement is finished and the sensor goes back in sleep mode automatically
- until next the next measurement is finished, the data can be read

To calculate temperature, pressure and humidity from the raw sensor data the 32bit algorithm given by Bosch in the data sheet is implemented. For energy saving, no oversampling ("single") is used as recommended for weather monitoring.

## Programming
The library provides the functions
1. `bool begin()` initialize the communication and sensor. Returns `true` if a BME280 or BMP280 sensor is found. Optionally, you can provide your own `TwoWire` instance to be used: `begin(TwoWire &wirePort)`. The latter makes most sense if you do not use standard I2C ports on the ESP-Plattform. 
2. `void setAddress(unit8_t address)` optionally set the I2C address of the sensor (default is `0x76`, you might try `setAddress(0x77)` if the sensor does not respond
3. `void reset()`reset sensor. Need to call `begin()` afterwards
4. `void startMeasurement()` kick off a measurement cycle. The sensor goes back to sleep automatically thereafter.
5. `bool isMeasuring()` returns `true` while a measurement is being performed.
6. `int32_t getTemperature()` returns the temperature in units of 0.01 DegC. An output value of "5123" equals to 51.23 DegC
7. `uint32_t getPressure()`returns the pressure in Pa. An output value of "96386" equals to 96386 Pa (or 963.86 hPa)
8. `uint32_t getHumidity()` returns the humidity in Q22.10 format (22 integer and 10 fractional bits). An output value of “47445” equals to 47445/1024 = 46. 333 %RH
The values for temperature, pressure and humidity are only read from the sensor once per measurement. Thus, subsequent calls to `getTemperature()` or the equivalents will return the identical values until a new measurement is started. As for pressure and humidity calculations the temperature value is needed, `getTemperature()` should be called first (however, the library internally keeps track and calculates the corresponding temperature in case you call the `get*()`-functions in a different order).

A typical measurement in the `loop()` would look like this (the complete sketch is in the [example folder](examples/Simple/Simple.ino)):
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
- The sensor will actually need a few moments before `isMeasuring()` returns `true`. In the above example, the meassage is printed once on an Arduino Pro Mini. On other boards your mileage may vary.
- If you call the `get*()`-functions before the measurement is finished, the sensor (and the thus the `get*()`-functions) will return the data from the last previously finished measurement. This may or may not be important, depending on the time between subsequent measurements.
- `get*()` will return the same value until the next time `startMeasurement()` is called (and the measurement is finished!).
