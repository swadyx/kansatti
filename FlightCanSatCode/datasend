#include "CanSatNeXT.h"
#include "definitions.h"

void getSendData(Measurements data, SCD40Data data, MQSensorData data, GPSData data){
  float ldr = data.board.ldr // mainboard built-in sensors data
  float temperature = data.board.temperature
  float pressure = data.board.pressure
  float acceleration = data.board.acceleration

  float temperaturescd40 = data.scd40.temperature // scd40 sensor datas
  uint16_t co2Concentration = data.scd40.co2Concentration
  float relativehumidity = data.scd40.relativeHumidity
  int error = data.scd40.error

  int mq135 = data.mq.mq135  // mq-sensors data
  int mq4 = data.mq.mq4
  
  bool updatedData = data.gps.dataUpdated // gps sensor data
  double lat = data.gps.latitude
  double lon = data.gps.longitude
  double alt = data.gps.altitude
  double spd = data.gps.speed
  uint8_t stlts = data.gps.satellites
  int hour = data.gps.hour
  int min = data.gps.minute
  int sec = data.gps.second
