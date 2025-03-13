

const String states = "/states.csv";

Measurements get_measurements () {
  Measurements data;
  data.scd40= get_scd40_data();
  if (data.scd40.error != NO_ERROR) {
    Serial.print("SCD40 data not available... error code: ");
    Serial.println(data.scd40.error);
  }
  data.mq = get_mq_sensor_data();
  data.gps = get_gps_data();
  if (!data.gps.dataUpdated) {
    Serial.print("New gps data not available!");
  }
  data.board = get_board_sensor_data();

  return data;
}


void sendMeasurements(Measurements meas) {
//   // Construct a comma-separated message with a unique prefix "MEAS:"
  String message = "MEAS:"; 
  message += String(meas.scd40.co2Concentration) + ",";
  message += String(meas.scd40.temperature) + ",";
  message += String(meas.scd40.relativeHumidity) + ",";
  message += String(meas.scd40.error) + ",";
  
  message += String(meas.mq.mq135) + ",";
  message += String(meas.mq.mq4) + ",";
  
//   // // gps.dataUpdated is a boolean; converting to int (0 or 1)
  message += String(meas.gps.dataUpdated ? 1 : 0) + ",";
  message += String(meas.gps.latitude, 6) + ",";
  message += String(meas.gps.longitude, 6) + ",";
  message += String(meas.gps.altitude) + ",";
  message += String(meas.gps.speed) + ",";
  message += String(meas.gps.satellites) + ",";
  message += String(meas.gps.hour) + ",";
  message += String(meas.gps.minute) + ",";
  message += String(meas.gps.second) + ",";
  
  message += String(meas.board.temperature) + ",";
  message += String(meas.board.pressure) + ",";
  message += String(meas.board.ldr) + ",";
  message += String(meas.board.acceleration) + ",";
  message += String(meas.board.acc_x) + ",";
  message += String(meas.board.acc_y) + ",";
  message += String(meas.board.acc_z);
  
//   // Transmit the complete measurements message
  sendData(message);
}

void sendGPS(GPSData gps) {
//   // Construct a comma-separated message with a unique prefix "MEAS:"
  String message = "GPS:"; 
  
//   // // gps.dataUpdated is a boolean; converting to int (0 or 1)
  message += String(gps.dataUpdated ? 1 : 0) + ",";
  message += String(gps.latitude, 6) + ",";
  message += String(gps.longitude, 6) + ",";
  message += String(gps.altitude) + ",";
  message += String(gps.speed) + ",";
  message += String(gps.satellites) + ",";
  message += String(gps.hour) + ",";
  message += String(gps.minute) + ",";
  message += String(gps.second) + ",";
  
  sendData(message);
}

void blinkLED()
{
  if(LED_IS_ON)
  {
    digitalWrite(LED, LOW);
  }else{
    digitalWrite(LED, HIGH);
  }
  LED_IS_ON = !LED_IS_ON;
}

void onDataReceived(String data) {
  Serial.println(data);
  
  if (data == "PRELAUNCH") {
    STATE = 0;
    writeFile(states, String(State));

  }
  else if (data == "FLIGHT") {
    STATE = 1;
    writeFile(states, String(STATE));
  }
  else if (data == "RECOVERY") {
    STATE = 2;
    writeFile(states, String(STATE));
  }
  else if (data == "INIT_SCD40") {
    logAndSend("Initializing SCD40 sensor...");
    if (init_scd40_sensor()) {
      logAndSend("SCD40 sensor initialization complete");
    } else {
      logAndSend("SCD40 sensor initialization failed");
    }
  }
  else if (data == "INIT_GPS") {
    logAndSend("Initializing GPS sensor...");
    if (init_gps_sensor()) {
      logAndSend("GPS sensor initialization complete");
    } else {
      logAndSend("GPS sensor initialization failed");
    }
  }
  else if (data == "INIT_MQ") {
    logAndSend("Initializing MQ sensors...");
    if (init_mq_sensor()) {
      logAndSend("MQ sensors initialization complete");
    } else {
      logAndSend("MQ sensors initialization failed");
    }
  }
  else if (data == "INIT_BOARD") {
    logAndSend("Initializing board sensors...");
    if (init_board_sensor()) {
      logAndSend("Board sensors initialization complete");
    } else {
      logAndSend("Board sensors initialization failed");
    }
  }
}
