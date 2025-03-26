#include "definitions.h"

void setup_mq_sensors() {  
  pinMode(27, OUTPUT);
  pinMode(32, INPUT);
  pinMode(33, INPUT);
  digitalWrite(27, HIGH);
}

void desetup_mq_sensors() {  
  digitalWrite(27, LOW);
}

// test the mq_sensors
bool test_mq_sensors() {
  setup_mq_sensors();
  MQSensorData mq = get_mq_sensor_data();
  
  if (mq.mq4 != 0) {
    sendData("MQ4 working: " + String(mq.mq4));
  } else {
    while (true) {
      sendData("MQ4 not working!");
      delay(1000);
    }
  }
  
  if (mq.mq135 != 0) {
    sendData("MQ135 working: " + String(mq.mq135));
  } else {
    sendData("MQ135 not working!");
  }
  
  return true;
}

MQSensorData get_mq_sensor_data() {
  MQSensorData data;
  data.mq135 = analogRead(33);
  data.mq4 = analogRead(32);
  return data;
}
