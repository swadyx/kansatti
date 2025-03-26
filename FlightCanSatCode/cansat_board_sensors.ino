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

bool test_board_sensors() {
  BoardSensorsData board = get_board_sensor_data();
  
  if (board.ldr > 0) {
    sendData("LDR-sensor working: " + String(board.ldr));
  } else {
    while (true) {
      sendData("LDR sensor not working!");
      delay(1000);
    }
  }
  
  if (board.temperature > -50 && board.temperature < 60) {
    sendData("Board temp-sensor working: " + String(board.temperature));
  } else {
    while (true) {
      sendData("Board temp-sensor not working! Temperature: " + String(board.temperature));
      delay(1000);
    }
  }
  
  if (board.pressure > 80000 && board.pressure < 120000) {
    sendData("Board pressure-sensor working: " + String(board.pressure));
  } else {
    while (true) {
      sendData("Board pressure-sensor not working!");
      delay(1000);
    }
  }
  
  if (board.acceleration != 0) {
    sendData("Board acceleration-sensor working: " + String(board.acceleration));
  } else {
    while (true) {
      sendData("Board acceleration-sensor not working!");
      delay(1000);
    }
  }
  
  return true;
}
