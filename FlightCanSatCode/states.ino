#include "definitions.h"

void prelaunch_mode() {
  blinkLED();
  delay(2000);
}

void flight_mode(){
  Measurements data = get_measurements();
  writeFile(LAUNCH_TIME_FILE, data.mission_time_s*1000);
  int write_error = save_data(data);
  int transmission_error = sendMeasurements(data);
  blinkLED();
  delay(TIME_BETWEEN_MEASUREMENTS);
  if (data.mission_time_s > CHANGE_TO_RECOVERY_AFTER_S) {
    desetup_mq_sensors();
    STATE = 2;
    writeFile(STATE_FILE, STATE);
    writeFile(LAUNCH_TIME_FILE, "0");
  } 
}


void recovery_mode()
{
  GPSData gps = get_gps_data();
  if (!gps.dataUpdated) {
    Serial.print("New gps data not available!");
    sendData("New gps data not available");
  } else {
    sendGPS(gps);
  }
  blinkLED();
  delay(5000);
}
