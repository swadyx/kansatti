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
const String allDataFile = "/all_data.csv";  // New combined data file

// Variable to track mission time in seconds
unsigned long missionStartTime = 0;
bool missionStarted = false;

void savedata(Measurements data) {
  // Initialize mission time if not started
  if (!missionStarted) {
    missionStartTime = millis();
    missionStarted = true;

    // Create header for the all_data file
    appendFile(allDataFile, "Time(s):Pressure(hPa):BoardTemp(C):LDR:Accel(g):"
                            "SCD40Temp(C):CO2(ppm):Humidity(%):MQ135:MQ4:"
                            "Lat:Lon:Alt(m):Speed(km/h):Satellites\n");
  }

  // Calculate mission time in seconds
  unsigned long currentTime = millis();
  float missionTimeSeconds = (currentTime - missionStartTime) / 1000.0;

  float ldr = data.board.ldr;  // mainboard built-in sensors data
  float temperature = data.board.temperature;
  float pressure = data.board.pressure;
  float acceleration = data.board.acceleration;

  float temperaturescd40 = data.scd40.temperature;  // scd40 sensor datas
  uint16_t co2Concentration = data.scd40.co2Concentration;
  float relativehumidity = data.scd40.relativeHumidity;

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
  int error = data.scd40.error;
  Serial.println(co2Concentration);

  // Save individual data files as before
  appendFile(mq135File, String(mq135) + "\n");
  appendFile(mq4File, String(mq4) + "\n");

  appendFile(ldrFile, String(ldr, 2) + "\n");
  appendFile(boardTemperatureFile, String(temperature, 2) + "\n");
  appendFile(pressureFile, String(pressure, 2) + "\n");
  appendFile(accelerationFile, String(acceleration, 2) + "\n");

  if (error != 1) {
    appendFile(co2ConcentrationFile, String(co2Concentration) + "\n");
    appendFile(relativeHumidityFile, String(relativehumidity, 2) + "\n");
    appendFile(SCD40temperatureFile, String(temperaturescd40, 2) + "\n");
    sendData("New SCD40 data successfully written into file!");
  } else {
    sendData("Error in SCD40: No new data found!");
  }

  // Create combined data string with all measurements
  String allDataString =
    String(missionTimeSeconds, 1) + ":" + String(pressure, 2) + ":" + String(temperature, 2) + ":" + String(ldr, 2) + ":" + String(acceleration, 2) + ":";

  // Add SCD40 data (use placeholder values if error)
  if (error != 1) {
    allDataString +=
      String(temperaturescd40, 2) + ":" + String(co2Concentration) + ":" + String(relativehumidity, 2) + ":";
  } else {
    allDataString += "NA:NA:NA:";
  }

  // Add MQ sensor data
  allDataString +=
    String(mq135) + ":" + String(mq4) + ":";

  // Add GPS data (use placeholder values if not updated)
  if (updatedData) {
    allDataString +=
      String(lat, 6) + ":" + String(lon, 6) + ":" + String(alt, 2) + ":" + String(spd, 2) + ":" + String(stlts);
  } else {
    allDataString += "NA:NA:NA:NA:NA";
  }

  // Append the combined data to the all_data file
  appendFile(allDataFile, allDataString + "\n");

  sendData("Data saved successfully!");
}
