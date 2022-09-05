# pocketBME280

There are many libraries to handle communication with the Bosch BME280 within Arduino code out there. But all of them did not fully fit my needs. I wanted something small (optionally using integer arithmetics alone) and only measuring data on demand. On top, the interface should be as simple as possible to use.

Basically, the library implements the measurement using the "forced mode". This means:

- by default the sensor is in sleep mode using very little power
- on demand a measurement is started
- after about 10ms, the measurement is finished and the sensor goes back in sleep mode automatically
- until next the next measurement is finished, the data can be read

To calculate temperature, pressure and humidity from the raw sensor data the 32 bit algorithm given by Bosch in the data sheet is used. To save energy, no oversampling is used.
