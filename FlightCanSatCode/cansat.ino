#include <Adafruit_MCP3008.h>
#include "CanSatNeXT.h"
#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "definitions.h"
#include <math.h>

void setup() {
  // put your setup code here, to run once:
  // Initialize serial communication for debugging
  CanSatInit(100);
  Serial.begin(115200);
  setup_scd40();
  setup_mq_sensors();
}

void loop() {

  SCD40Data SCD40SensorData = get_scd40_data();
  if (SCD40SensorData.error == NO_ERROR) {
    Serial.print("CO2 concentration [ppm]: ");
    Serial.println(SCD40SensorData.co2Concentration);
    Serial.print("Temperature [°C]: ");
    Serial.println(SCD40SensorData.temperature);
    Serial.print("Relative Humidity [%RH]: ");
    Serial.println(SCD40SensorData.relativeHumidity);
  } else {
    Serial.print("SCD40 data not available... error code: ");
    Serial.println(SCD40SensorData.error);
  }

  MQSensorData MQData = get_mq_sensor_data();
  Serial.println();
  Serial.print("MQ 135:");
  Serial.println(MQData.mq135);
  Serial.print("MQ 4:");
  Serial.println(MQData.mq4);

  GPSData gpsData = get_gps_data();
  
  // Only print if new GPS data was updated
  if (gpsData.dataUpdated) {
    Serial.print("Latitude: ");
    Serial.println(gpsData.latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(gpsData.longitude, 6);
    Serial.print("Altitude: ");
    Serial.println(gpsData.altitude);
    Serial.print("Speed: ");
    Serial.println(gpsData.speed);
    Serial.print("Satellites: ");
    Serial.println(gpsData.satellites);
    Serial.print("Time: ");
    Serial.print(gpsData.hour);
    Serial.print(":");
    Serial.print(gpsData.minute);
    Serial.print(":");
    Serial.println(gpsData.second);
    Serial.println("-------------------");
  }

  BoardSensorsData boardData = get_board_sensor_data();
  Serial.println(boardData.acceleration);
  Serial.println(boardData.temperature);
  Serial.println(boardData.ldr);
  Serial.println(boardData.pressure);
  
  delay(6000);
}
