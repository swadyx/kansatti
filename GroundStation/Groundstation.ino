#include "CanSatNeXT.h"

void setup() {
  Serial.begin(115200);
  GroundStationInit(100);

}

void loop() {}

void onDataReceived(String data)
{
  Serial.println(data);

}

