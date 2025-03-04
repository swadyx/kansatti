Measurements get_measurements () {
  Measurements data;
  data.scd40= get_scd40_data();
  // if (data.scd40.error != NO_ERROR) {
  //   Serial.print("SCD40 data not available... error code: ");
  //   Serial.println(data.scd40.error);
  // }
  data.mq = get_mq_sensor_data();
  data.gps = get_gps_data();
  // if (!data.gps.dataUpdated) {
  //   Serial.print("New gps data not available!");
  // }
  data.board = get_board_sensor_data();

  return data;
}

// ADD REDUNDANCY!!!!!!!!!!!!!!!!!
bool detect_liftoff() {
  float accel = get_acc();
  if (accel > LIFTOFF_ACCEL_THRESHOLD) {
    Serial.println("Liftoff detected via accelerometer.");
    return true;
  }
  return false;
}

bool detect_landing() {
  return false;
}

void save_data(Measurements data) {

}
  
void transmit_data(Measurements data) {

}
