#include "definitions.h"

Measurements get_measurements () {
  Measurements data;
  data.scd40= get_scd40_data();
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


int sendMeasurements(Measurements meas) {
//   // Construct a comma-separated message with a unique prefix "MEAS:"
  String message = "MEAS:"; 
  message += String(meas.scd40.co2Concentration) + ",";
  message += String(meas.scd40.temperature) + ",";
  message += String(meas.scd40.relativeHumidity) + ",";
  message += String(meas.scd40.error) + ",";
  
  message += String(meas.mq.mq135) + ",";
  message += String(meas.mq.mq4) + ",";
  
//   // // gps.dataUpdated is a boolean; converting to int (0 or 1)
  message += String(meas.gps.dataUpdated ? 1 : 0) + ",";
  message += String(meas.gps.latitude, 6) + ",";
  message += String(meas.gps.longitude, 6) + ",";
  message += String(meas.gps.altitude) + ",";
  message += String(meas.gps.speed) + ",";
  message += String(meas.gps.satellites) + ",";
  message += String(meas.gps.hour) + ",";
  message += String(meas.gps.minute) + ",";
  message += String(meas.gps.second) + ",";
  
  message += String(meas.board.temperature) + ",";
  message += String(meas.board.pressure) + ",";
  message += String(meas.board.ldr) + ",";
  message += String(meas.board.acceleration) + ",";
  message += String(meas.board.acc_x) + ",";
  message += String(meas.board.acc_y) + ",";
  message += String(meas.board.acc_z);
  
//   // Transmit the complete measurements message
  return sendData(message);
}

int sendGPS(GPSData gps) {
//   // Construct a comma-separated message with a unique prefix "MEAS:"
  String message = "GPS:"; 
  
//   // // gps.dataUpdated is a boolean; converting to int (0 or 1)
  message += String(gps.dataUpdated ? 1 : 0) + ",";
  message += String(gps.latitude, 6) + ",";
  message += String(gps.longitude, 6) + ",";
  message += String(gps.altitude) + ",";
  message += String(gps.speed) + ",";
  message += String(gps.satellites) + ",";
  message += String(gps.hour) + ",";
  message += String(gps.minute) + ",";
  message += String(gps.second) + ",";
  
  return sendData(message);
}

void blinkLED()
{
  if(LED_IS_ON)
  {
    digitalWrite(LED, LOW);
  }else{
    digitalWrite(LED, HIGH);
  }
  LED_IS_ON = !LED_IS_ON;
}

void onDataReceived(String data) {
  Serial.println(data);
  
  if (data == "PRELAUNCH") {
    STATE = 0;
    writeFile(STATE_FILE, STATE);
    writeFile(LAUNCH_TIME_FILE, "0");
  }
  else if (data == "FLIGHT") {
    setup_mq_sensors();
    LAUNCH_TIME = millis();
    STATE = 1;
    writeFile(STATE_FILE, STATE);
  }
  else if (data == "RECOVERY") {
    desetup_mq_sensors();
    STATE = 2;
    writeFile(STATE_FILE, STATE);
    writeFile(LAUNCH_TIME_FILE, "0");
  }
  else if (data == "INIT_SCD40") {
    sendData("Initializing SCD40 sensor...");
    if (init_scd40_sensor()) {
      sendData("SCD40 sensor initialization complete");
    } else {
      sendData("SCD40 sensor initialization failed");
    }
  }
  else if (data == "INIT_GPS") {
    sendData("Initializing GPS sensor...");
    if (init_gps_sensor()) {
      sendData("GPS sensor initialization complete");
    } else {
      sendData("GPS sensor initialization failed");
    }
  }
  else if (data == "INIT_MQ") {
    sendData("Initializing MQ sensors...");
    if (init_mq_sensor()) {
      sendData("MQ sensors initialization complete");
    } else {
      sendData("MQ sensors initialization failed");
    }
  }
  else if (data == "INIT_BOARD") {
    sendData("Initializing board sensors...");
    if (init_board_sensor()) {
      sendData("Board sensors initialization complete");
    } else {
      sendData("Board sensors initialization failed");
    }
  } else if (data == "DEINIT_MQ") {
    sendData("Deinitializing MQ sensors...");
    desetup_mq_sensors();
  } else if (data == "RESET") {
    sendData("RESET");
    abort();
  } else {
    sendData("Unknown/corrupted command!");
  }
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