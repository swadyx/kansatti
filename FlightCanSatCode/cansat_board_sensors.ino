#include "definitions.h"

BoardSensorsData get_board_sensor_data() {
  BoardSensorsData data;
  data.ldr = analogReadVoltage(LDR);
  data.temperature = readTemperature();
  data.pressure = readPressure()*100; 
  float ax, ay, az;
  readAcceleration(ax, ay, az);
  float totalSquared = ax*ax+ay*ay+az*az;
  data.acceleration = sqrt(totalSquared);
  data.acc_x = ax;
  data.acc_y = ay;
  data.acc_z = az;

  return data;

}

float get_acc() {
  float ax, ay, az;
  readAcceleration(ax, ay, az);
  return sqrt(ax*ax+ay*ay+az*az);
}