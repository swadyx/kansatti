#ifndef DEFINITIONS_H 
#define DEFINITIONS_H


// Global state variables 
int STATE =                     0; // Prelaunch
unsigned long LAUNCH_TIME =     0;
unsigned long LAUNCH_TIME_TO_ADD=0; // for unexpected resets: adds time to the current time so that time is correctly calculated
float LIFTOFF_ACCEL_THRESHOLD = 1.8; // < 2
int SEARCH_TIME =               0;
bool LED_IS_ON =                false;
int TIME_BETWEEN_MEASUREMENTS = 800; // in ms
int CHANGE_TO_RECOVERY_AFTER_S    = 300; // in s
const String DATA_FILE =        "/data.csv"; 
const String STATE_FILE =       "/state.txt";
const String LAUNCH_TIME_FILE = "/launch_time.txt";

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
  float ldr;
  float acceleration;
  float acc_x;
  float acc_y;
  float acc_z;
};

struct Measurements {
  SCD40Data scd40;
  MQSensorData mq;
  GPSData gps;
  BoardSensorsData board;
  float mission_time_s;
};

#endif // DEFINITIONS_H