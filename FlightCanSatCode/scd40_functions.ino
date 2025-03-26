// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cScd4x sensor;
static int16_t error;

bool setup_scd40() {
  Wire.begin(21, 22); // SDA on GPIO 21, SCL on GPIO 22
  sensor.begin(Wire, SCD41_I2C_ADDR_62);

  uint64_t serialNumber = 0;
  delay(30);
  // Ensure sensor is in clean state
  error = sensor.wakeUp();
  if (error != NO_ERROR) {
    return false;
  }
  error = sensor.stopPeriodicMeasurement();
  if (error != NO_ERROR) {
    return false;
  }
  error = sensor.reinit();
  if (error != NO_ERROR) {
    return false;
  }
  error = sensor.getSerialNumber(serialNumber);
  if (error != NO_ERROR) {
    return false;
  }
  
  error = sensor.startPeriodicMeasurement();
  if (error != NO_ERROR) {
    return false;
  }

  return true;
}

bool test_scd40_sensor() {
  // Loop until a valid SCD40 reading is obtained
  SCD40Data scd40;
  while (true) {
    scd40 = get_scd40_data();
    if (scd40.error == NO_ERROR) {
      sendData("SCD40 working: " + String(scd40.co2Concentration) + " ppm");
      break;
    } else {
      sendData("SCD40 data not available... error code: " + String(scd40.error));
      delay(3000);
    }
  }
  return true;
}

SCD40Data get_scd40_data() {
  SCD40Data data;
  bool dataReady = false;

  // Check if data is ready
  data.error = sensor.getDataReadyStatus(dataReady);
  if (data.error != NO_ERROR) {
    return data;
  }
  if (!dataReady) {
    // assign a custom error code if no data is available
    data.error = 1;
    return data;
  }

  // Read the measurement and store values in the structure
  data.error = sensor.readMeasurement(data.co2Concentration, data.temperature, data.relativeHumidity);
  return data;
}
