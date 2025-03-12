#include "CanSatNeXT.h"
#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "definitions.h"

// Global state variables
int STATE = 0; // Prelaunch
long LAUNCH_TIME = 0; // Time
float LIFTOFF_ACCEL_THRESHOLD = 1.5; 
int SEARCH_TIME = 0;
bool LED_IS_ON = false;

// Sensor initialization flags
bool SCD40_INITIALIZED    = false;
bool GPS_INITIALIZED      = false;
bool MQ4_INITIALIZED      = false;
bool MQ135_INITIALIZED    = false;
bool LDR_INITIALIZED      = false;
bool BOARD_TEMP_INITIALIZED     = false;
bool BOARD_PRESSURE_INITIALIZED = false;
bool BOARD_ACCEL_INITIALIZED    = false;

void setup() {
  CanSatInit(28);
  Serial.begin(115200);  // for debugging (if needed)
  sendData("CANSAT ON!");
  setup_neo6m();
}

// This helper sends the message via sendData() and also prints it to the Serial Monitor.
void logAndSend(String message) {
  sendData(message);
  Serial.println(message);
}

bool init_scd40_sensor() {
  if (setup_scd40()) {
    logAndSend("SCD40 initialized!");
    SCD40_INITIALIZED = true;
  } else {
    logAndSend("SCD40 not working!");
    return false;
  }
  
  // Loop until a valid SCD40 reading is obtained
  SCD40Data scd40;
  while (true) {
    scd40 = get_scd40_data();
    if (scd40.error == NO_ERROR) {
      logAndSend("SCD40 working!");
      break;
    } else {
      logAndSend("SCD40 data not available... error code: " + String(scd40.error));
      delay(3000);
    }
  }
  return true;
}

bool init_gps_sensor() {
  unsigned long startTime = millis();
  while (millis() - startTime < 30000) {  // Wait at most 30 seconds
    GPSData gpsData = get_gps_data();
    if (gpsData.dataUpdated) {
      logAndSend("GPS-fix acquired!");
      GPS_INITIALIZED = true;
      return true;
    }
    logAndSend("Waiting for a GPS fix...");
    delay(1000);
  }
  logAndSend("GPS initialization timed out!");
  return false;
}

bool init_mq_sensor() {
  setup_mq_sensors();
  MQSensorData mq = get_mq_sensor_data();
  
  if (mq.mq4 != 0) {
    logAndSend("MQ4 working!");
    MQ4_INITIALIZED = true;
  } else {
    while (true) {
      logAndSend("MQ4 not working!");
      delay(1000);
    }
  }
  
  if (mq.mq135 != 0) {
    logAndSend("MQ135 working!");
    MQ135_INITIALIZED = true;
  } else {
    logAndSend("MQ135 not working!");
  }
  
  return true;
}

bool init_board_sensor() {
  BoardSensorsData board = get_board_sensor_data();
  
  if (board.ldr > 0) {
    logAndSend("LDR-sensor working!");
    LDR_INITIALIZED = true;
  } else {
    while (true) {
      logAndSend("LDR sensor not working!");
      delay(1000);
    }
  }
  
  if (board.temperature > -50 && board.temperature < 60) {
    logAndSend("Board temp-sensor working!");
    BOARD_TEMP_INITIALIZED = true;
  } else {
    while (true) {
      logAndSend("Board temp-sensor not working! Temperature: " + String(board.temperature));
      delay(1000);
    }
  }
  
  if (board.pressure > 80000 && board.pressure < 120000) {
    logAndSend("Board pressure-sensor working!");
    BOARD_PRESSURE_INITIALIZED = true;
  } else {
    while (true) {
      logAndSend("Board pressure-sensor not working!");
      delay(1000);
    }
  }
  
  if (board.acceleration != 0) {
    logAndSend("Board acceleration-sensor working!");
    BOARD_ACCEL_INITIALIZED = true;
  } else {
    while (true) {
      logAndSend("Board acceleration-sensor not working!");
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