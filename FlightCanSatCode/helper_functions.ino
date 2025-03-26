#include "definitions.h"

Measurements get_measurements () {
  Measurements data;
  data.scd40 = get_scd40_data();
  if (data.scd40.error != NO_ERROR) {
    Serial.print("SCD40 data not available... error code: ");
    Serial.println(data.scd40.error);
  }
  data.mq = get_mq_sensor_data();
  data.gps = get_gps_data();
  if (!data.gps.dataUpdated) {
    Serial.print("New gps data not available!");
  }
  data.board = get_board_sensor_data();
  data.mission_time_s = ((millis()+LAUNCH_TIME_TO_ADD) - LAUNCH_TIME) / 1000.0;
  return data;
}

int save_data(Measurements data) {
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
  float mission_time_s = data.mission_time_s;

  // Create combined data string with all measurements
  String allDataString =
    String(mission_time_s, 1) + ":" + String(pressure, 2) + ":" + String(temperature, 2) + ":" + String(ldr, 2) + ":" + String(acceleration, 2) + ":";
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
  return appendFile(DATA_FILE, allDataString + "\n");
}

void blinkLED() {
  if(LED_IS_ON)
  {
    digitalWrite(LED, LOW);
  }else{
    digitalWrite(LED, HIGH);
  }
  LED_IS_ON = !LED_IS_ON;
}

bool test_sd() {
  writeFile("/sd_test.txt", "SD TOIMII!!!");
  String contents = readFile("/sd_test.txt");
  writeFile("/sd_test.txt", "");
  if (contents == "SD TOIMII!!!") {
    return true;
  } else {
    return false;
  }
}