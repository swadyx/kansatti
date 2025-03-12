#include "CanSatNeXT.h"
#include "definitions.h"

const String gpsFile = "/gps_data.csv";
const String SCD40temperatureFile = "/temperature.csv";
const String ldrFile = "/ldr.csv";
const String boardTemperatureFile = "/board_temp.csv";
const String pressureFile = "/pressure.csv";
const String accelerationFile = "/acceleration.csv";
const String co2ConcentrationFile = "/co2.csv";
const String relativeHumidityFile = "/humidity.csv";
const String mq135File = "/mq135.csv";
const String mq4File = "/mq4.csv";



void savedata(Measurements data) {
  float ldr = data.board.ldr;  // mainboard built-in sensors data
  float temperature = data.board.temperature;
  float pressure = data.board.pressure;
  float acceleration = data.board.acceleration;

  float temperaturescd40 = data.scd40.temperature;  // scd40 sensor datas
  uint16_t co2Concentration = data.scd40.co2Concentration;
  float relativehumidity = data.scd40.relativeHumidity;
  int error = data.scd40.error;

  int mq135 = data.mq.mq135;  // mq-sensors data
  int mq4 = data.mq.mq4;

  bool updatedData = data.gps.dataUpdated;  // gps sensor data
  double lat = data.gps.latitude;
  double lon = data.gps.longitude;
  double alt = data.gps.altitude;
  double spd = data.gps.speed;
  uint8_t stlts = data.gps.satellites;
  int hour = data.gps.hour;
  int min = data.gps.minute;
  int sec = data.gps.second;

  appendFile(SCD40temperatureFile, String(temperature, 2) + "\n");
  appendFile(ldrFile, String(ldr, 2) + "\n");
  appendFile(boardTemperatureFile, String(temperature, 2) + "\n");
  appendFile(pressureFile, String(pressure, 2) + "\n");
  appendFile(accelerationFile, String(acceleration, 2) + "\n");
  appendFile(co2ConcentrationFile, String(co2Concentration, 2) + "\n");
  appendFile(relativeHumidityFile, String(relativehumidity, 2) + "\n");
  appendFile(mq135File, String(mq135, 2) + "\n");
  appendFile(mq4File, String(mq4, 2) + "\n");



}
