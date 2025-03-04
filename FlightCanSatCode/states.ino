void prelaunch() {
  if (detect_liftoff()) {
    LAUNCH_TIME = millis();
    STATE = 1; // flight
    Serial.println("Transitioning to FLIGHT mode.");
  }
}

// check for landing (HOW?)
// collect all data
// save all data to sd card
// transmit all data
void flight_mode() {
  Measurements data = get_measurements();
  save_data(data);
  transmit_data(data);
  if (detect_landing()) {
    STATE = 2; // landing
    Serial.println("Transitioning to RECOVERY mode.");
  }
}

// transmit gps-data
void recovery_mode() {

}