#include "CanSatNeXT.h"

#ifndef DEFINITIONS_H 
#define DEFINITIONS_H

const String datafile = "/data.csv";

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

struct BoardSensorsData {
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
  unsigned long timestamp;  // Added timestamp field
};

#endif // DEFINITIONS_H

// Global variables for mission time tracking
unsigned long launchTime = 0;
bool flightModeActive = false;

// Helper function to print a Measurements structure with timestamp
void printMeasurements(const Measurements& meas) {
  // Print with timestamp and individual labels
  Serial.println("Received Measurements:");
  
  Serial.print("SCD40 CO2: ");
  Serial.println(meas.scd40.co2Concentration);
  Serial.print("SCD40 Temperature: ");
  Serial.println(meas.scd40.temperature);
  Serial.print("SCD40 Humidity: ");
  Serial.println(meas.scd40.relativeHumidity);
  Serial.print("SCD40 Error: ");
  Serial.println(meas.scd40.error);
  
  Serial.print("MQ Sensor MQ135: ");
  Serial.println(meas.mq.mq135);
  Serial.print("MQ Sensor MQ4: ");
  Serial.println(meas.mq.mq4);
  
  Serial.print("GPS Data Updated: ");
  Serial.println(meas.gps.dataUpdated);
  Serial.print("GPS Latitude: ");
  Serial.println(meas.gps.latitude, 6);
  Serial.print("GPS Longitude: ");
  Serial.println(meas.gps.longitude, 6);
  Serial.print("GPS Altitude: ");
  Serial.println(meas.gps.altitude);
  Serial.print("GPS Speed: ");
  Serial.println(meas.gps.speed);
  Serial.print("GPS Satellites: ");
  Serial.println(meas.gps.satellites);
  Serial.print("GPS Time: ");
  Serial.print(meas.gps.hour);
  Serial.print(":");
  Serial.print(meas.gps.minute);
  Serial.print(":");
  Serial.println(meas.gps.second);
  
  Serial.print("Board Temperature: ");
  Serial.println(meas.board.temperature);
  Serial.print("Board Pressure: ");
  Serial.println(meas.board.pressure);
  Serial.print("Board LDR: ");
  Serial.println(meas.board.ldr);
  Serial.print("Board Acceleration: ");
  Serial.println(meas.board.acceleration);
  Serial.print("Board Acc X: ");
  Serial.println(meas.board.acc_x);
  Serial.print("Board Acc Y: ");
  Serial.println(meas.board.acc_y);
  Serial.print("Board Acc Z: ");
  Serial.println(meas.board.acc_z);
  
  // Also send a single CSV line for easy logging, including mission time
  Serial.print("CSV:");
  Serial.print(meas.timestamp);
  Serial.print(",");
  Serial.print(flightModeActive ? (meas.timestamp - launchTime) : 0);  // Mission time
  Serial.print(",");
  Serial.print(meas.scd40.co2Concentration);
  Serial.print(",");
  Serial.print(meas.scd40.temperature);
  Serial.print(",");
  Serial.print(meas.scd40.relativeHumidity);
  Serial.print(",");
  Serial.print(meas.mq.mq135);
  Serial.print(",");
  Serial.print(meas.mq.mq4);
  Serial.print(",");
  Serial.print(meas.gps.latitude, 6);
  Serial.print(",");
  Serial.print(meas.gps.longitude, 6);
  Serial.print(",");
  Serial.print(meas.gps.altitude);
  Serial.print(",");
  Serial.print(meas.board.temperature);
  Serial.print(",");
  Serial.print(meas.board.pressure);
  Serial.print(",");
  Serial.print(meas.board.ldr);
  Serial.print(",");
  Serial.print(meas.board.acceleration);
  Serial.print(",");
  Serial.print(meas.board.acc_x);
  Serial.print(",");
  Serial.print(meas.board.acc_y);
  Serial.print(",");
  Serial.println(meas.board.acc_z);
}

// Helper function to parse a comma-separated measurements payload
bool parseMeasurements(String payload, Measurements &meas) {
  const int expectedTokens = 22;
  int startIdx = 0;
  int commaIdx;
  String token;
  
  // Add timestamp when parsing measurements
  meas.timestamp = millis();
  
  for (int i = 0; i < expectedTokens; i++) {
    commaIdx = payload.indexOf(',', startIdx);
    if (commaIdx == -1 && i < expectedTokens - 1) {
      Serial.println("Error: Not enough data fields");
      return false;
    }
    token = (i < expectedTokens - 1)
              ? payload.substring(startIdx, commaIdx)
              : payload.substring(startIdx);
    startIdx = (i < expectedTokens - 1) ? commaIdx + 1 : startIdx;
    
    switch (i) {
      case 0:  meas.scd40.co2Concentration = token.toInt();   break;
      case 1:  meas.scd40.temperature      = token.toFloat(); break;
      case 2:  meas.scd40.relativeHumidity = token.toFloat(); break;
      case 3:  meas.scd40.error            = token.toInt();   break;
      case 4:  meas.mq.mq135               = token.toInt();   break;
      case 5:  meas.mq.mq4                 = token.toInt();   break;
      case 6:  meas.gps.dataUpdated        = (token.toInt() != 0); break;
      case 7:  meas.gps.latitude           = token.toFloat(); break;
      case 8:  meas.gps.longitude          = token.toFloat(); break;
      case 9:  meas.gps.altitude           = token.toFloat(); break;
      case 10: meas.gps.speed              = token.toFloat(); break;
      case 11: meas.gps.satellites         = token.toInt();   break;
      case 12: meas.gps.hour               = token.toInt();   break;
      case 13: meas.gps.minute             = token.toInt();   break;
      case 14: meas.gps.second             = token.toInt();   break;
      case 15: meas.board.temperature      = token.toFloat(); break;
      case 16: meas.board.pressure         = token.toFloat(); break;
      case 17: meas.board.ldr              = token.toFloat(); break;
      case 18: meas.board.acceleration     = token.toFloat(); break;
      case 19: meas.board.acc_x            = token.toFloat(); break;
      case 20: meas.board.acc_y            = token.toFloat(); break;
      case 21: meas.board.acc_z            = token.toFloat(); break;
    }
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  GroundStationInit(28);
}

void loop() {
  if (Serial.available() > 0) {
    String receivedMessage = Serial.readStringUntil('\n'); 
    Serial.print("Command to transmit: ");
    Serial.println(receivedMessage);
    
    // Check if the command is to enter flight mode
    if (receivedMessage == "FLIGHT" && !flightModeActive) {
      launchTime = millis();
      flightModeActive = true;
      Serial.println("LAUNCH_TIME_SET");
    } 
    // Check if the command is to exit flight mode
    else if (receivedMessage == "RECOVERY" || receivedMessage == "PRELAUNCH") {
      flightModeActive = false;
    }
    
    sendData(receivedMessage);  
  }
}

void onDataReceived(String data)
{
  // Add timestamp to all received data
  unsigned long currentMillis = millis();
  
  // Process state message
  if (data.startsWith("STATE: ")) {
    String stateStr = data.substring(7);
    stateStr.trim();
    int state = stateStr.toInt();
    
    // Check if entering FLIGHT mode
    if (state == 1 && !flightModeActive) {
      launchTime = currentMillis;
      flightModeActive = true;
      Serial.println("LAUNCH_TIME_SET");
    } else if (state != 1) {
      flightModeActive = false;
    }
    
    Serial.print("TIME:");
    Serial.print(currentMillis);
    Serial.print(" MISSION_TIME:");
    Serial.print(flightModeActive ? (currentMillis - launchTime) : 0);
    Serial.print(" STATE:");
    if (state == 0) {
      Serial.println("prelaunch");
    } else if (state == 1) {
      Serial.println("flight");
    } else if (state == 2) {
      Serial.println("recovery");
    }
  }
  // Process measurement data
  else if (data.startsWith("MEAS:")) {
    String payload = data.substring(5); // Remove "MEAS:" prefix
    Measurements meas;
    if (parseMeasurements(payload, meas)) {
      printMeasurements(meas);
    }
  }
  // Handle other data
  else {
    Serial.print("TIME:");
    Serial.print(currentMillis);
    Serial.print(" MISSION_TIME:");
    Serial.print(flightModeActive ? (currentMillis - launchTime) : 0);
    Serial.print(" MSG:");
    Serial.println(data);
  }
}
