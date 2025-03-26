#include "definitions.h"

int sendMeasurements(Measurements meas) {
  // Construct a comma-separated message with a unique prefix "MEAS:"
  String message = "MEAS:"; 
  message += String(meas.scd40.co2Concentration) + ",";
  message += String(meas.scd40.temperature) + ",";
  message += String(meas.scd40.relativeHumidity) + ",";
  message += String(meas.scd40.error) + ",";
  message += String(meas.mq.mq135) + ",";
  message += String(meas.mq.mq4) + ",";
  // gps.dataUpdated is a boolean; converting to int (0 or 1)
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

  return sendData(message);
}

int sendGPS(GPSData gps) {
  String message = "GPS:"; 
  // gps.dataUpdated is a boolean; converting to int (0 or 1)
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

void onDataReceived(String data) {
  Serial.println(data);
  
  if (data == "PRELAUNCH") {
    STATE = 0;
    writeFile(STATE_FILE, STATE);
    writeFile(LAUNCH_TIME_FILE, "0");
  }
  else if (data == "FLIGHT") {
    setup_mq_sensors();
    LAUNCH_TIME_MS = millis();
    STATE = 1;
    writeFile(STATE_FILE, STATE);
  }
  else if (data == "RECOVERY") {
    desetup_mq_sensors();
    STATE = 2;
    writeFile(STATE_FILE, STATE);
    writeFile(LAUNCH_TIME_FILE, "0");
  }
  else if (data == "TEST_SCD40") {
    if (test_scd40_sensor()) {
      sendData("SCD40 sensor working!");
    } else {
      sendData("ERROR: SCD40 sensor not working!");
    }
  }
  else if (data == "TEST_GPS") {
    if (test_gps_sensor()) {
      sendData("NEO6M has a gps fix!");
    } else {
      sendData("ERROR: NEO6M couldn't find any a gps signal!");
    }
  }
  else if (data == "TEST_MQ") {
    if (test_mq_sensors()) {
      sendData("MQ sensors working!");
    } else {
      sendData("ERROR: MQ sensors not working!");
    }
  }
  else if (data == "TEST_BOARD") {
    if (test_board_sensors()) {
      sendData("Board sensors working!");
    } else {
      sendData("ERROR: Board sensors not working!");
    }
  }
  else if (data == "TEST_SD") {
    if (test_sd()) {
      sendData("SD write and read operations working!");
    } else {
      sendData("SD not working!");
    }
  }
  else if (data == "MQ_ON") {
    sendData("Turning on MQ sensors...");
    setup_mq_sensors();
  } 
  else if (data == "MQ_OFF") {
    sendData("Turning off MQ sensors...");
    desetup_mq_sensors();
  } else if (data == "RESET") {
    sendData("RESET");
    abort();
  } else {
    sendData("Unknown/corrupted command: " + String(data));
  }
}