#include <Adafruit_MCP3008.h>
#include "CanSatNeXT.h"
#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "SD.h"
#include "definitions.h"

int STATE = 0; // Prelaunch
long LAUNCH_TIME = 0; // Time
float LIFTOFF_ACCEL_THRESHOLD = 1.8;

void setup() {
  CanSatInit(100);
  Serial.begin(115200);
  if (!setup_data_file()) {
    while (true) {
      setup_data_file();
      Serial.println("SD-card initialization failed!");
      delay(3000);
    }
  }
  setup_scd40();
  setup_mq_sensors();
  setup_neo6m();

  // check that the gps is working
  // while (true) {
  //   GPSData gpsData = get_gps_data();
  //   if (gpsData.dataUpdated) {
  //     Serial.println("GPS-fix acquired!");
  //     break;
  //   }
  //   Serial.println("Waiting for a gps fix...");
  //   delay(3000);
  // }

  // check that scd40 is collecting data.
  SCD40Data scd40;
  while (true) {
    scd40 = get_scd40_data();
    if (scd40.error == NO_ERROR) {
      Serial.println("SCD40 working!");
      break;
    } else {
      Serial.print("SCD40 data not available... error code: ");
      Serial.println(scd40.error);
      delay(3000);
    }
  }

  // check that board sensors are working.
  BoardSensorsData board = get_board_sensor_data();
  if (board.ldr > 0) {
    Serial.print("LDR-sensor working!");
  } else {
    while (true) {
      Serial.println("LDR sensor not working!");
    }
  }
  if (board.temperature > -50 &&  board.temperature < 60) {
    Serial.print("Board temp-sensor working!");
  } else {
    while (true) {
      Serial.println("Board temp-sensor not working!");
      Serial.println(board.temperature);
      delay(1000);
    }
  }
  if (board.pressure > 80000 &&  board.pressure < 120000) {
    Serial.print("Board pressure-sensor working!");
  } else {
    while (true) {
      Serial.println("Board pressure-sensor not working!");
    }
  }
  if (board.acceleration != 0) {
    Serial.print("Board acceleration-sensor working!");
  } else {
    while (true) {
      Serial.println("Board acceleration-sensor not working!");
    }
  }

  // check that mq-sensors are working
  MQSensorData mq = get_mq_sensor_data();
  if (mq.mq4 == 0) {
    Serial.print("MQ4 working!");
  } else {
    while (true) {
      Serial.println("MQ4 not working!");
    }
  }
  if (mq.mq135 == 0) {
    Serial.print("MQ135 working!");
  } else {
    while (true) {
      Serial.println("MQ135 not working!");
    }
  }
}

void loop() {
  if(STATE == 0)
  {
    prelaunch();
  }else if(STATE == 1)
  {
    flight_mode();
  }else if(STATE == 2){
    recovery_mode();
  }else{
    Serial.println("What the sigma?");
    delay(1000);
  }
}
