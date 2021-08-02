

byte sensorsInit()
{
  byte good = 1;
  // ***** INPUT *****
  // Start up the TempSensor library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  good = sensors.getDeviceCount() >= 4;

  int i;  // "i" was resetting in the second loop below.  Safer to make it local.
  for (i = 0; i < min(MAX_SENSORS, sensors.getDeviceCount()); i++) {
    if (!sensors.getAddress(Thermometer[i], i)) {
      Serial.print("Unable to find address for Tank ");
      Serial.println(i);
      good = 0;
    }
  }

  // show the addresses we found on the bus, and set all resolutions to 12 bit
  for (i = 0; i < min(MAX_SENSORS, sensors.getDeviceCount()); i++) {
    Serial.print("Tank "); Serial.print(i); Serial.print(" Address: ");
    printAddress(Thermometer[i]);
    Serial.println();
    sensors.setResolution(Thermometer[i], 12);
  }
  return good;
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

/**
 * Get the current temperatures for the sensors.
 * FOR TESTING ONLY, generate some printable but obviously wrong temps if there are no sensors.
 */
void updateTemps() {
  byte i;
  if (sensors.getDeviceCount() == 0) {
    for (i=0; i<4; i++) {
      TempInput[i] = 10.0 + i*10.0;
    }
  } else {
    sensors.requestTemperatures(); // Send the command to get temperatures
    for (i = 0; i < sensors.getDeviceCount(); i++) {
      TempInput[i] = sensors.getTempCByIndex(i);
      if (TempInput[i] < 0.0 || TempInput[i] > 80.0) {
        Serial.print("BAD  temperature on sensor "); Serial.println(i);
      } else {
         //Serial.print("Good temperature on sensor ");
      }
    }
  }
}

/** 
 *  Save just time and temperature for easier parsing when graphing temperatures on the fly.
 */
void saveTemps() {
  logFile = SD.open("GRAPHPTS.TXT", FILE_WRITE);
  logFile.print(millis()); logFile.print(",");
  Serial.print("Saved "); Serial.println(millis());
  for (byte i=0; i<4; i++) {
    logFile.print(TempInput[i], 1); logFile.print(",");
  }
  logFile.println();
  logFile.close();
}
/**
 * Delete the plotting file on restart for easier testing.  Note that
 * we may not want this during "real" runs, but we'll have to watch of for
 * non-monotonic times.
 */
void clearTemps() {
  SD.remove("GRAPHPTS.TXT");
}
