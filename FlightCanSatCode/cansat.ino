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
int LIFTOFF_ACCEL_THRESHOLD = 1.5; // Päivitetty arvo 1.8 -> 1.5 ja float -> int
int searchTime = 0;

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

  // GPS-fixin hakeminen
  while (true) {
    GPSData gpsData = get_gps_data();
    if (gpsData.dataUpdated) {
      Serial.println("GPS-fix acquired!");
      Serial.print("Total search time: ");
      Serial.print(searchTime);
      Serial.println(" seconds");
      break;
    }
    
    searchTime++;
    Serial.print("Waiting for a GPS fix... ");
    Serial.print(searchTime);
    Serial.println(" seconds elapsed");
    char info[50];
    sprintf(info, "Data not found after waiting %d seconds", searchTime);
    sendData(info);
    delay(1000);
  }

  // SCD40-sensorin tarkistus
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

  // Board-sensoreiden tarkistus
  BoardSensorsData board = get_board_sensor_data();
  if (board.ldr > 0) {
    Serial.println("LDR-sensor working!");
  } else {
    while (true) {
      Serial.println("LDR sensor not working!");
    }
  }
  if (board.temperature > -50 && board.temperature < 60) {
    Serial.println("Board temp-sensor working!");
  } else {
    while (true) {
      Serial.println("Board temp-sensor not working!");
      Serial.println(board.temperature);
      delay(1000);
    }
  }
  if (board.pressure > 80000 && board.pressure < 120000) {
    Serial.println("Board pressure-sensor working!");
  } else {
    while (true) {
      Serial.println("Board pressure-sensor not working!");
    }
  }
  if (board.acceleration != 0) {
    Serial.println("Board acceleration-sensor working!");
  } else {
    while (true) {
      Serial.println("Board acceleration-sensor not working!");
    }
  }

  // MQ-sensorien tarkistus
  MQSensorData mq = get_mq_sensor_data();
  if (mq.mq4 != 0) { // Muutettu logiikka (ennen == 0)
    Serial.println("MQ4 working!");
  } else {
    while (true) {
      Serial.println("MQ4 not working!");
    }
  }
  if (mq.mq135 != 0) { // Muutettu logiikka (ennen == 0)
    Serial.println("MQ135 working!");
  } else {
    while (true) {
      Serial.println("MQ135 not working!");
    }
  }
}

void loop() {
  if (STATE == 0) {
    prelaunch();
  } else if (STATE == 1) {
    flight_mode();
  } else if (STATE == 2) {
    recovery_mode();
  } else {
    Serial.println("What the sigma?");
    delay(1000);
  }
}
