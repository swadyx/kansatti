void prelaunch_mode() {
  blinkLED();
  delay(2000);
}

void flight_mode(){
  float LDR_voltage = analogReadVoltage(LDR);
  Measurements data = get_measurements();
  sendMeasurements(data);
  // sendData("DATA" + String(LDR_voltage));
  savedata(data);
  blinkLED();
  delay(1000);

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
