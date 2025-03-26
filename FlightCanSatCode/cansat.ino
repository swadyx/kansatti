#include "CanSatNeXT.h"
#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "definitions.h"

void setup() {

  CanSatInit(28);
  Serial.begin(115200);  // for debugging (if needed)
  sendData("CANSAT ON!");
  setup_neo6m();
  if (!setup_scd40()) {
    sendData("SCD40 not working!");
  }

  // the following is for unexpected resets
  String prev_state_str = readFile(STATE_FILE);
  prev_state_str.trim();
  if (prev_state_str.toInt(); == 1) {  //flight mode
    LAUNCH_TIME = millis();
    String launch_time_to_add_str = readFile(LAUNCH_TIME_FILE); // read this from a file
    launch_time_to_add_str.trim();
    LAUNCH_TIME_TO_ADD = launch_time_to_add_str.toInt();
    STATE = 1;
    sendData("Mid-flight reset detected! Continuing...");
    setup_mq_sensors();
  // only do the following if the last time the cansat was powerd off it wasn't in flight mode
  } else {
    int write_error = writeFile(DATA_FILE, "Time(s):Pressure(hPa):BoardTemp(C):LDR:Accel(g):" // cant do this, what if cansat resets
                            "SCD40Temp(C):CO2(ppm):Humidity(%):MQ135:MQ4:"
                            "Lat:Lon:Alt(m):Speed(km/h):Satellites\n");
    sendData("SD returned: " + String(write_error));
  }
}

void loop() {
  sendData("STATE: " + String(STATE));
  if (STATE == 0) {
    prelaunch_mode();
  } else if (STATE == 1) {
    flight_mode();
  } else if (STATE == 2) {
    recovery_mode();
  }
}