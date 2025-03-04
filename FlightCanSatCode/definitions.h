#ifndef DEFINITIONS_H 
#define DEFINITIONS_H

TinyGPSPlus gps;

struct MQSensorData {
  int mq135;
  int mq4;
};

struct SCD40Data {
  uint16_t co2Concentration;
  float temperature;
  float relativeHumidity;
  int error; // error code: NO_ERROR (0) means success; non-zero indicates an error
};

struct GPSData {
  bool dataUpdated;     // Indicates if new GPS data is available
  double latitude;
  double longitude;
  double altitude;
  double speed;
  uint8_t satellites;
  int hour;
  int minute;
  int second;
};
struct BoardSensorsData{
  float temperature;
  float pressure;
  int ldr;
  int acceleration;
  int acc_x;
  int acc_y;
  int acc_z;
};
#endif // DEFINITIONS_H
