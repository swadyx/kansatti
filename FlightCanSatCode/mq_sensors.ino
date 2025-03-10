#include "definitions.h"

#define ADC_CS    12    // Chip Select for MCP3008
// Declare an instance of the MCP3008 object
Adafruit_MCP3008 adc;


void setup_mq_sensors() {  
  // Initialize the MCP3008 using the provided line:
  adc.begin(SPI_CLK, SPI_MOSI, SPI_MISO, ADC_CS);
}

MQSensorData get_mq_sensor_data() {
  MQSensorData data;
  data.mq135 = adc.readADC(0);
  data.mq4 = adc.readADC(1);
  data.mq135 = 10;
  data.mq4 = 10;
  
  return data;
}
