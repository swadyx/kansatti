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
    Serial.println(accel);
    return true;
  }
  return false;
}

bool detect_landing() {
  return false;
}

bool setup_data_file() {
  // Check if an SD-card is present
  if (!SDCardPresent()) {
    Serial.println("No sd-card found!");
    return false;
  }
  
  // Define header for CSV file
  String header = "co2Concentration,scd40_temperature,scd40_relativeHumidity,scd40_error,"
                  "mq135,mq4,dataUpdated,latitude,longitude,altitude,speed,satellites,"
                  "hour,minute,second,board_temperature,board_pressure,board_ldr,"
                  "board_acceleration,board_acc_x,board_acc_y,board_acc_z\n";
  
  // If the file doesn't exist, write the header to initialize it
  if (!fileExists("/measurements.csv")) {
    uint8_t status = writeFile("/measurements.csv", header);
    Serial.println(status);
    if (status != 0) {
      return false;
    }
  }
  return true;
}

bool save_data(Measurements data) {
  // Check if an SD-card is present
  if (!SDCardPresent()) {
    return false;
  }
  
  String record = "";
  
  // SCD40 sensor data: if error equals 1, store zeros for these values.
  if (data.scd40.error == 1) {
    record += "0,0,0,0,";
  } else {
    record += String(data.scd40.co2Concentration) + ",";
    record += String(data.scd40.temperature) + ",";
    record += String(data.scd40.relativeHumidity) + ",";
    record += String(data.scd40.error) + ",";
  }
  
  // MQ sensor data
  record += String(data.mq.mq135) + ",";
  record += String(data.mq.mq4) + ",";
  
  // GPS data: if not updated, store zeros for all GPS fields.
  if (data.gps.dataUpdated) {
    record += "1,";  // Indicate data updated
    record += String(data.gps.latitude) + ",";
    record += String(data.gps.longitude) + ",";
    record += String(data.gps.altitude) + ",";
    record += String(data.gps.speed) + ",";
    record += String(data.gps.satellites) + ",";
    record += String(data.gps.hour) + ",";
    record += String(data.gps.minute) + ",";
    record += String(data.gps.second) + ",";
  } else {
    // Data not updated, write zeros for each GPS field (9 fields)
    record += "0,0,0,0,0,0,0,0,0,";
  }
  
  // Board sensor data
  record += String(data.board.temperature) + ",";
  record += String(data.board.pressure) + ",";
  record += String(data.board.ldr) + ",";
  record += String(data.board.acceleration) + ",";
  record += String(data.board.acc_x) + ",";
  record += String(data.board.acc_y) + ",";
  record += String(data.board.acc_z) + "\n";
  
  // Append the record to "measurements.csv"
  uint8_t status = appendFile("measurements.csv", record);
  
  // Return true if the write operation was successful (status==0)
  return (status == 0);
}
  
void transmit_data(Measurements data) {

}
