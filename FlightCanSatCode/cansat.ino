#include "CanSatNeXT.h"
#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "definitions.h"

void setup() {

  CanSatInit(28);
  Serial.begin(115200);  // for debugging (if needed)
  sendData("CANSAT ON!");
  setup_neo6m();
  if (!setup_scd40()) {
    sendData("SCD40 not working!");
  }

  // the following line is for unexpected resets
  String prev_state_str = readFile(STATE_FILE);
  prev_state_str.trim();
  int prev_state = prev_state_str.toInt();
  if (prev_state == 1) {  //flight mode
    LAUNCH_TIME = millis();
    String launch_time_to_add_str = readFile(LAUNCH_TIME_FILE); // read this from a file
    launch_time_to_add_str.trim();
    LAUNCH_TIME_TO_ADD = launch_time_to_add_str.toInt();
    STATE = 1;
    sendData("Mid-flight reset detected! Continuing...");
    setup_mq_sensors();

  } else {
    int write_error = writeFile(DATA_FILE, "Time(s):Pressure(hPa):BoardTemp(C):LDR:Accel(g):" // cant do this, what if cansat resets
                            "SCD40Temp(C):CO2(ppm):Humidity(%):MQ135:MQ4:"
                            "Lat:Lon:Alt(m):Speed(km/h):Satellites\n");
    sendData("SD returned: " + String(write_error));
  }
}

bool init_scd40_sensor() {
  // Loop until a valid SCD40 reading is obtained
  SCD40Data scd40;
  while (true) {
    scd40 = get_scd40_data();
    if (scd40.error == NO_ERROR) {
      sendData("SCD40 working!");
      break;
    } else {
      sendData("SCD40 data not available... error code: " + String(scd40.error));
      delay(3000);
    }
  }
  SCD40_INITIALIZED = true;
  return true;
}

bool init_gps_sensor() {
  unsigned long startTime = millis();
  while (millis() - startTime < 30000) {  // Wait at most 30 seconds
    GPSData gpsData = get_gps_data();
    if (gpsData.dataUpdated) {
      sendData("GPS-fix acquired!");
      GPS_INITIALIZED = true;
      return true;
    }
    sendData("Waiting for a GPS fix...");
    delay(1000);
  }
  sendData("GPS initialization timed out!");
  return false;
}

bool init_mq_sensor() {
  setup_mq_sensors();
  MQSensorData mq = get_mq_sensor_data();
  
  if (mq.mq4 != 0) {
    sendData("MQ4 working!");
    MQ4_INITIALIZED = true;
  } else {
    while (true) {
      sendData("MQ4 not working!");
      delay(1000);
    }
  }
  
  if (mq.mq135 != 0) {
    sendData("MQ135 working!");
    MQ135_INITIALIZED = true;
  } else {
    sendData("MQ135 not working!");
  }
  
  return true;
}

bool init_board_sensor() {
  BoardSensorsData board = get_board_sensor_data();
  
  if (board.ldr > 0) {
    sendData("LDR-sensor working!");
    LDR_INITIALIZED = true;
  } else {
    while (true) {
      sendData("LDR sensor not working!");
      delay(1000);
    }
  }
  
  if (board.temperature > -50 && board.temperature < 60) {
    sendData("Board temp-sensor working!");
    BOARD_TEMP_INITIALIZED = true;
  } else {
    while (true) {
      sendData("Board temp-sensor not working! Temperature: " + String(board.temperature));
      delay(1000);
    }
  }
  
  if (board.pressure > 80000 && board.pressure < 120000) {
    sendData("Board pressure-sensor working!");
    BOARD_PRESSURE_INITIALIZED = true;
  } else {
    while (true) {
      sendData("Board pressure-sensor not working!");
      delay(1000);
    }
  }
  
  if (board.acceleration != 0) {
    sendData("Board acceleration-sensor working!");
    BOARD_ACCEL_INITIALIZED = true;
  } else {
    while (true) {
      sendData("Board acceleration-sensor not working!");
      delay(1000);
    }
  }
  
  return true;
}

void loop() {
  sendData("STATE: " + String(STATE));
  if (STATE == 0) {
    prelaunch_mode();
  } else if (STATE == 1) {
    flight_mode();
  } else if (STATE == 2) {
    recovery_mode();
  }
}