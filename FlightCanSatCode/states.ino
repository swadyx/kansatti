#include "definitions.h"

void prelaunch_mode() {
  blinkLED();
  delay(2000);
}

void flight_mode() {
  Measurements data = get_measurements();
  writeFile(LAUNCH_TIME_FILE, data.mission_time_ms);
  save_data(data);
  sendMeasurements(data);
  blinkLED();
  delay(TIME_BETWEEN_MEASUREMENTS);
}

void recovery_mode() {
  GPSData gps = get_gps_data();
  if (!gps.dataUpdated) {
    sendData("New gps data not available");
  } else {
    sendGPS(gps);
  }
  blinkLED();
  delay(5000);
}
