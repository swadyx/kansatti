#include "CanSatNeXT.h"
#include <Adafruit_MCP3008.h>

#define ADC_CS    12    // Chip Select for MCP3008
// Declare an instance of the MCP3008 object
Adafruit_MCP3008 adc;

void setup() {
  // Initialize serial communication for debugging
  CanSatInit(100);
  Serial.begin(115200);
  
  // Initialize the MCP3008 using the provided line:
  adc.begin(SPI_CLK, SPI_MOSI, SPI_MISO, ADC_CS);
}

void loop() {
  // Read the value from channel 0 where the MQ sensor is connected
  int sensorValue1 = adc.readADC(0); // Change the channel if necessary
  int sensorValue2 = adc.readADC(1); // Change the channel if necessary
  
  // Print the sensor value to the Serial Monitor
  Serial.print("MQ Sensor Values: ");
  Serial.println(sensorValue1);
  Serial.println(sensorValue2);
  
  // Wait 1 second before taking the next reading
  delay(1000);
}
