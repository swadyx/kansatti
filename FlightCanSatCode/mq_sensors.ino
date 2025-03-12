#include "definitions.h"

void setup_mq_sensors() {  
  // Initialize the MCP3008 using the provided line:
  // adc.begin(SPI_CLK, SPI_MOSI, SPI_MISO, ADC_CS);
  pinMode(27, OUTPUT);
  pinMode(32, INPUT);
  pinMode(33, INPUT);
  digitalWrite(27, HIGH);
}

MQSensorData get_mq_sensor_data() {
  MQSensorData data;
  data.mq135 = analogRead(32);
  data.mq4 = analogRead(33);
  return data;
}
