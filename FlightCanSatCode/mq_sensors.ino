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

MQSensorData get_mq_sensor_data() {
  MQSensorData data;
  data.mq135 = analogRead(32);
  data.mq4 = analogRead(33);
  return data;
}
