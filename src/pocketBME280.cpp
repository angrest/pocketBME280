//****************************************************************************//
//
//  pocketBME280 Library
//  Axel Grewe - angrest@gmx.net 5-Sep-2022
//  Read all sensor data in a single burst and then calculate consistent values from buffered raw data
//  Use int32 math only for compatibility, only basic functionality for size optimization
//
//****************************************************************************//

#include "pocketBME280.h"

//Constructor -- Specifies default configuration
pocketBME280::pocketBME280(void) {
  //Construct with these default settings
  _i2CAddress = 0x76;   // Default, jumper open is 0x77
  _sensorPort = &Wire;  //Default to Wire port
}


//****************************************************************************//
//
//  Configuration section
//
//****************************************************************************//
bool pocketBME280::begin() {
  delay(2);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

  _t_fine = INT32_MAX;
  _temperature = INT32_MAX;
  _pressure = UINT32_MAX;
  _humidity = UINT32_MAX;

  //Check communication with IC before anything else
  uint8_t chipID = readRegister(BME280_CHIP_ID_REG);  //Should return 0x60 or 0x58
  if (chipID != 0x58 && chipID != 0x60)               // Is this BMP or BME?
    return (chipID == 0x58) || (chipID == 0x60);      //This is not BMP nor BME!

  //Reading all compensation data, range 0x88:A1, 0xE1:E7
  compensation.dig_T1 = ((uint16_t)((readRegister(BME280_DIG_T1_MSB_REG) << 8) + readRegister(BME280_DIG_T1_LSB_REG)));
  compensation.dig_T2 = ((int16_t)((readRegister(BME280_DIG_T2_MSB_REG) << 8) + readRegister(BME280_DIG_T2_LSB_REG)));
  compensation.dig_T3 = ((int16_t)((readRegister(BME280_DIG_T3_MSB_REG) << 8) + readRegister(BME280_DIG_T3_LSB_REG)));

  compensation.dig_P1 = ((uint16_t)((readRegister(BME280_DIG_P1_MSB_REG) << 8) + readRegister(BME280_DIG_P1_LSB_REG)));
  compensation.dig_P2 = ((int16_t)((readRegister(BME280_DIG_P2_MSB_REG) << 8) + readRegister(BME280_DIG_P2_LSB_REG)));
  compensation.dig_P3 = ((int16_t)((readRegister(BME280_DIG_P3_MSB_REG) << 8) + readRegister(BME280_DIG_P3_LSB_REG)));
  compensation.dig_P4 = ((int16_t)((readRegister(BME280_DIG_P4_MSB_REG) << 8) + readRegister(BME280_DIG_P4_LSB_REG)));
  compensation.dig_P5 = ((int16_t)((readRegister(BME280_DIG_P5_MSB_REG) << 8) + readRegister(BME280_DIG_P5_LSB_REG)));
  compensation.dig_P6 = ((int16_t)((readRegister(BME280_DIG_P6_MSB_REG) << 8) + readRegister(BME280_DIG_P6_LSB_REG)));
  compensation.dig_P7 = ((int16_t)((readRegister(BME280_DIG_P7_MSB_REG) << 8) + readRegister(BME280_DIG_P7_LSB_REG)));
  compensation.dig_P8 = ((int16_t)((readRegister(BME280_DIG_P8_MSB_REG) << 8) + readRegister(BME280_DIG_P8_LSB_REG)));
  compensation.dig_P9 = ((int16_t)((readRegister(BME280_DIG_P9_MSB_REG) << 8) + readRegister(BME280_DIG_P9_LSB_REG)));

  compensation.dig_H1 = ((uint8_t)(readRegister(BME280_DIG_H1_REG)));
  compensation.dig_H2 = ((int16_t)((readRegister(BME280_DIG_H2_MSB_REG) << 8) + readRegister(BME280_DIG_H2_LSB_REG)));
  compensation.dig_H3 = ((uint8_t)(readRegister(BME280_DIG_H3_REG)));
  compensation.dig_H4 = ((int16_t)((readRegister(BME280_DIG_H4_MSB_REG) << 4) + (readRegister(BME280_DIG_H4_LSB_REG) & 0x0F)));
  compensation.dig_H5 = ((int16_t)((readRegister(BME280_DIG_H5_MSB_REG) << 4) + ((readRegister(BME280_DIG_H4_LSB_REG) >> 4) & 0x0F)));
  compensation.dig_H6 = ((int8_t)readRegister(BME280_DIG_H6_REG));

  writeRegister(BME280_CTRL_HUMIDITY_REG, 1);                             // enable measurement of humitidity
  writeRegister(BME280_CTRL_MEAS_REG, (1 << 5) | (1 << 2) | MODE_SLEEP);  // enable temperature and pressure measurement, activate humidity measurement

  return (chipID == 0x58) || (chipID == 0x60);
}

//Begin comm with BME280 over I2C
bool pocketBME280::begin(TwoWire &wirePort) {
  _sensorPort = &wirePort;

  return begin();
}


void pocketBME280::setAddress(uint8_t newAddress) {
  _i2CAddress = newAddress;
}

//Reset sensor. Need to call .begin() afterwards
void pocketBME280::reset(void) {
  writeRegister(BME280_RST_REG, 0xB6);
}

//****************************************************************************
//
//  Temperature Calculation - 32bit variant
//  Output in 0.01 DegC. Output value of "5123" equals to 51.23 DegC
//  t_fine is a global variable used in calculation of Hunidity and pressure
//  Data Sheet chapter 4.2.3
//
//****************************************************************************
int32_t pocketBME280::compensateTemperature(int32_t adc_T) {

  int32_t var1, var2, T;

  var1 = ((((adc_T >> 3) - ((int32_t)compensation.dig_T1 << 1))) * ((int32_t)compensation.dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((int32_t)compensation.dig_T1)) * ((adc_T >> 4) - ((int32_t)compensation.dig_T1))) >> 12) * ((int32_t)compensation.dig_T3)) >> 14;
  _t_fine = var1 + var2;
  T = (_t_fine * 5 + 128) >> 8;

  return T;
}

// wrapper function for usability
int32_t pocketBME280::getTemperature(void) {

  if (_t_fine == INT32_MAX) {
    readBurst();  // read out sensor
  }
  if (_temperature == INT32_MAX) {
    int32_t adc_T = ((uint32_t)_burstBuffer[3] << 12) | ((uint32_t)_burstBuffer[4] << 4) | ((_burstBuffer[5] >> 4) & 0x0F);
    _temperature = compensateTemperature(adc_T);  // Output value of "5123" equals to 51.23 DegC
  }
  return _temperature;
}

//****************************************************************************
//
//  Pressure Calculation - 32bit variant
//  Accuracy of 1 Pa typically (1-sigma)
//  Output in Pa. Output value of "96386" equals to 96386 Pa = 963.86 hPa
//  Data Sheet chapter 8.2
//
//****************************************************************************
uint32_t pocketBME280::compensatePressure(int32_t adc_P) {

  int32_t var1, var2;
  uint32_t p;
  var1 = (((int32_t)_t_fine) >> 1) - (int32_t)64000;
  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)compensation.dig_P6);
  var2 = var2 + ((var1 * ((int32_t)compensation.dig_P5)) << 1);
  var2 = (var2 >> 2) + (((int32_t)compensation.dig_P4) << 16);
  var1 = (((compensation.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)compensation.dig_P2) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) * ((uint32_t)compensation.dig_P1)) >> 15);

  if (var1 == 0)
    return 0;  // avoid exception caused by division by zero

  p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
  if (p < 0x80000000)
    p = (p << 1) / ((uint32_t)var1);
  else
    p = (p / (uint32_t)var1) * 2;
  var1 = (((int32_t)compensation.dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
  var2 = (((int32_t)(p >> 2)) * ((int32_t)compensation.dig_P8)) >> 13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + compensation.dig_P7) >> 4));

  return p;
}

// wrapper function for usability
uint32_t pocketBME280::getPressure(void) {

  if (_t_fine == INT32_MAX) {
    getTemperature();  // initialize t_fine
  }

  if (_pressure == UINT32_MAX) {
    int32_t adc_P = ((uint32_t)_burstBuffer[0] << 12) | ((uint32_t)_burstBuffer[1] << 4) | ((_burstBuffer[2] >> 4) & 0x0F);
    _pressure = compensatePressure(adc_P);  // Output value of "96386" equals to 96386 Pa
  }
  return _pressure;
}

//****************************************************************************//
//
//  Humidity Calculation
//  Set humidity in %RH as unsigned 32 bit integer in Q22. 10 format (22 integer and 10 fractional bits).
//  Output value of “47445” represents 47445/1024 = 46. 333 %RH
//  Data Sheet Chapter 4.2.3
//
//****************************************************************************//
uint32_t pocketBME280::compensateHumidity(int32_t adc_H) {

  int32_t v_x1_u32r;
  v_x1_u32r = (_t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)compensation.dig_H4) << 20) - (((int32_t)compensation.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)compensation.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)compensation.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)compensation.dig_H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)compensation.dig_H1)) >> 4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

  return (uint32_t)(v_x1_u32r >> 12);
}

// wrapper function for usability
uint32_t pocketBME280::getHumidity(void) {

  if (_t_fine == INT32_MAX) {
    getTemperature();  // initialize t_fine
  }
  if (_humidity == UINT32_MAX) {
    int32_t adc_H = ((uint32_t)_burstBuffer[6] << 8) | ((uint32_t)_burstBuffer[7]);
    _humidity = compensateHumidity(adc_H);  //Output value of “47445” represents 47445/1024 = 46. 333 %RH
  }
  return _humidity;
}

//****************************************************************************
//
//  Manage measurement
//
//****************************************************************************
void pocketBME280::startMeasurement() {
  uint8_t state = readRegister(BME280_CTRL_MEAS_REG);
  state &= ~((1 << 1) | (1 << 0));  //Clear the mode[1:0] bits
  state |= MODE_FORCED;             // start measurement, goes back to sleep automatically when finished
  writeRegister(BME280_CTRL_MEAS_REG, state);

  _t_fine = INT32_MAX;  // set t_fine to invalid value
  _temperature = INT32_MAX;
  _pressure = UINT32_MAX;
  _humidity = UINT32_MAX;
}

bool pocketBME280::isMeasuring() {
  uint8_t status = readRegister(BME280_STAT_REG);
  return (status & (1 << 3));  //If bit 3 is set, measurement is finished: return true
}


//****************************************************************************
//
//  Utility functions for communication via bus
//
//****************************************************************************
void pocketBME280::readBurst() {
  uint8_t *writePointer = _burstBuffer;
  uint8_t i = 0;
  uint8_t b = 0;
  static uint8_t numBytes = 8;

  _sensorPort->beginTransmission(_i2CAddress);
  _sensorPort->write(BME280_MEASUREMENTS_REG);
  _sensorPort->endTransmission();

  // request measurement data from sensor
  _sensorPort->requestFrom(_i2CAddress, numBytes);
  while ((_sensorPort->available()) && (i < numBytes)) {  // sensor may send less than requested

    b = _sensorPort->read();  // receive a byte
    *writePointer = b;
    writePointer++;
    i++;
  }
}

uint8_t pocketBME280::readRegister(uint8_t pointer) {

  uint8_t result;

  _sensorPort->beginTransmission(_i2CAddress);
  _sensorPort->write(pointer);
  _sensorPort->endTransmission();

  _sensorPort->requestFrom(_i2CAddress, (uint8_t)1);
  if (_sensorPort->available())
    result = _sensorPort->read();  // receive one byte
  else
    result = 0;

  return result;
}

void pocketBME280::writeRegister(uint8_t pointer, uint8_t value) {

  _sensorPort->beginTransmission(_i2CAddress);
  _sensorPort->write(pointer);
  _sensorPort->write(value);
  _sensorPort->endTransmission();
}