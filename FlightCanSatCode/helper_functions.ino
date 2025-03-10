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
  File file = SD.open("/data.csv", FILE_WRITE);
  // if (file) {
  //   return true;
  // }

  // Define header for CSV file
  String header = "co2_concentration,scd40_temperature,scd40_relative_humidity,scd40_error,"
                  "mq135,mq4,dataUpdated,latitude,longitude,altitude,speed,satellites,"
                  "hour,minute,second,board_temperature,board_pressure,board_ldr,"
                  "board_acceleration,board_acc_x,board_acc_y,board_acc_z\n";

  size_t expectedBytes = header.length();

  // Write the string and get the number of bytes written.
  size_t bytesWritten = file.print(header);

  // Check if the number of bytes written matches the string length.
  if (bytesWritten == expectedBytes) {
    Serial.println("Write success: all bytes written.");
    return true;
  } else {
    Serial.print("Write error: expected ");
    Serial.print(expectedBytes);
    Serial.print(" bytes, but only wrote ");
    Serial.println(bytesWritten);
    return false;
  }
}

bool save_data(Measurements data) {
   File file = SD.open("/data.csv", FILE_APPEND);
  // if (file) {
  //   return true;
  // }

  // Write the string and get the number of bytes written.
  size_t bytesWritten = file.print("ok mitä vituuuuuu\r\n");

  // Check if the number of bytes written matches the string length.
  if (bytesWritten > 0) {
    Serial.println("Write success: all bytes written.");
    return true;
  } else {
    return false;
  }
}

void send_data(Measurements data){
  float ldr = data.board.ldr; // mainboard built-in sensors data
  float temperature = data.board.temperature;
  float pressure = data.board.pressure;
  float acceleration = data.board.acceleration;

  float temperaturescd40 = data.scd40.temperature; // scd40 sensor datas
  uint16_t co2Concentration = data.scd40.co2Concentration;
  float relativehumidity = data.scd40.relativeHumidity;
  int error = data.scd40.error;

  int mq135 = data.mq.mq135;  // mq-sensors data
  int mq4 = data.mq.mq4;
  
  bool updatedData = data.gps.dataUpdated; // gps sensor data
  double lat = data.gps.latitude;
  double lon = data.gps.longitude;
  double alt = data.gps.altitude;
  double spd = data.gps.speed;
  uint8_t stlts = data.gps.satellites;
  int hour = data.gps.hour;
  int min = data.gps.minute;
  int sec = data.gps.second;

String dataPacket = String("Temperature: ") +
                    String(temperature) + ", Pressure: " + 
                    String(pressure) + ", LDR: " + 
                    String(ldr) + ",Acceleration: " + 
                    String(acceleration) + ",SCD40 Temp: " +
                    String(temperaturescd40) + ", CO2 concentration: " + 
                    String(co2Concentration) + ", Rel. Humid. : " + 
                    String(relativehumidity) + ", MQ135: " +
                    String(mq135) + ", MQ4: " + 
                    String(mq4) + ", Latitude: " +
                    String(lat, 6) + ", Longtitude: " + 
                    String(lon, 6) + ", Altitude: " + 
                    String(alt) + ", Speed: " +
                    String(spd) + ", Satellites: " + 
                    String(stlts) + ", HH:MM:SS --> " +
                    String(hour) + ":" + 
                    String(min) + ":" + 
                    String(sec);
  
sendData(dataPacket);
}

void only_gps_data(Measurements data){
  bool updatedData = data.gps.dataUpdated; // gps sensor data
  double lat = data.gps.latitude;
  double lon = data.gps.longitude;
  double alt = data.gps.altitude;
  double spd = data.gps.speed;
  uint8_t stlts = data.gps.satellites;
  int hour = data.gps.hour;
  int min = data.gps.minute;
  int sec = data.gps.second;

  String dataPacket1 = String(lat, 6) + "," + 
                      String(lon, 6) + "," + 
                      String(alt) + "," +
                      String(spd) + "," + 
                      String(stlts) + "," +
                      String(hour) + ":" + 
                      String(min) + ":" + 
                      String(sec);
  
  sendData(dataPacket1);
}

