/*
   This sketch is meant to be run on an Arduino Mega with new CBASS-R shield.
   The goal is to verify that all functions of the board which are significant
   to CBASS are correctly wired and are working.
   Manual checks before running the software
   - With power from USB 5V, 3.3V, and GND test points are correct
   - With power from input cable (middle connector, pin 3) all 4 test points are correct
   - Board reset button resets Arduino
   Software checks (some requiring hardware attached) include
   - display works for graphics and text
     - see if OLED and TFT code can be similar
   - clock can be set and read and holds time without power
   - temperature can be read and is reasonable
   - 12 relay outputs can be toggled (will require hardware to check)
   - SD card slot can be accessed and used
   - BLE card can be accessed and used
*/
/*
   Library Notes
   DallasTemperature 3.9.0 installed 6 Apr 2021 (was 3.8.0 in default directory and Code_Restart)
   OneWire 2.3.5 installed 6 Apr 2021 (was 2.3.4 in default directory, 2.2 in Code_Restart)
*/

// Arduino pins for the current configuration, with useful constants.
#include "Pins_Constants.h"

// SD card
#include <SD.h>

// Sensors
#include <OneWire.h>
#include <DallasTemperature.h>

// Clock
#include <RTClib.h>
RTC_DS1307  rtc;

// Display (see display.ino for dimensions, etc.)
#include <Adafruit_ILI9341.h>

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


// Bluetooth
#include "BluefruitConfig.h"
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

// Sensor related:
#define MAX_SENSORS 5
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(TempSensorPin);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
// arrays to hold device addresses
DeviceAddress Thermometer[MAX_SENSORS];
double TempInput[4];  // Latest temperature reading
unsigned long nextSave = 0;
// How often to save temperatures for plotting (ms). This should be kept longer than the time it takes for each loop().
unsigned long tSaveInterval = 10000;

// Relay pins
// XXX - be sure on and off have the same definition for the blue 12V relays and Robo-Tank!
#define RELAY_ON 1
#define RELAY_OFF 0

boolean relayState[3][4];
char tankNames[][2] = {"A", "B", "C", "D"};  // Initialization may be unnecessary.


File logFile;

// For Bluetooth
#define FACTORYRESET_ENABLE         1
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}
// These constants are from the fountain code.  They should be renamed or eliminated here.
// Warning: without the casts below, some sort of truncation causes the delay to fail.
const unsigned long checkDelay = (unsigned long)15 * (unsigned long)1000; // Units are milliseconds.
const unsigned long bleDelay = (unsigned long)1 * (unsigned long)200; // Units are milliseconds.

const char testName[6][10] = {"Display", "SD card", "Bluetooth", "Clock", "Sensors", "Relays"};

const byte DISP = 0;
const byte SDcard = 1;
const byte BLE = 2;
const byte CLOCK = 3;
const byte SENSOR = 4;
const byte RELAY = 5;

// SET THESE TRUE TO TEST EACH ITEM ABOVE
// SET FALSE IF THE CORRESPONDING PART IS NOT INSTALLED
const byte doTest[] = {true, true, false, true, true, true};
// Results of test (some always return true, for now!)
byte testResult[] = {0, 0, 0, 0, 0, 0};

// Other flags for identifying states
#define HEAT 0
#define CHILL 1
#define LIGHT 2

// It is nice to print a degree symbol, but the code is different on each device.
const char* deg_serial = "\xc2\xb0";
const char* deg_tft = "\xf7";

void setup() {
  Serial.begin(9600);

  startDisplay();

  delay(20);
  //CharacterGrid();
  //delay(3000);

  Serial.println("\n=== Starting CBASS-R Test===");

  if (doTest[BLE]) testResult[BLE] = bleSetup();

  if (doTest[DISP]) testResult[DISP] = tftHello();

  if (doTest[CLOCK]) testResult[CLOCK] = checkClock();

  if (doTest[SDcard]) testResult[SDcard] = trySD();

  if (doTest[SENSOR]) {
    testResult[SENSOR] = sensorsInit();
    showTemps();

  }

  if (doTest[RELAY]) {
    testResult[RELAY] = 1;  // No way to know if it worked.
    tft.fillScreen(BLACK);
    setRelayPins();
    flipRelays();
  }

  byte goodCount = 0;
  byte testCount = 0;
  Serial.println("==== Test Results ====");

  for (byte i = 0; i < sizeof(doTest) / sizeof(byte); i++) {
    if (doTest[i]) {
      testCount++;
      if (testResult[i]) {
        goodCount++;
        Serial.print("  Passed  ");
      } else {
        Serial.print("  Failed  ");
      }
    } else {
      Serial.print("  No test ");
    }
    Serial.println(testName[i]);
  }
  Serial.print("==== Passed "); Serial.print(goodCount); Serial.print(" of "); Serial.print(testCount); Serial.println(" tests. ====");
  Serial.println("==== Note that most tests always return good, and should be checked manually. ====");
  if (doTest[DISP]) {
    tft.setCursor(0, 0);
    tft.fillScreen(WHITE);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.print("Test Result ");
    if (goodCount == testCount) {
      tft.setTextColor(DARKGREEN);
    } else {
      tft.setTextColor(RED);
    }
    tft.print(goodCount); tft.print("/"); tft.println(testCount);
    tft.setTextColor(BLACK);
    tft.println("End of CBASS-R test setup phase");
    delay(2000);
    tft.fillScreen(BLACK);
  }

  Serial.println("End of CBASS-R setup phase.");
  nextSave = millis();  // This ensures that the loop does a save on the first pass.
  clearTemps(); // Don't reuse old plotting data.
}

void loop() {
  if (doTest[BLE] && ble.isConnected()) {
    interact();
  }
  updateTemps();
  showTemps();
  if (nextSave < millis()) {
    if (millis() - nextSave > tSaveInterval) {
      Serial.print("WARNING: planned graph interval is shorter than loop time!  Saving "); Serial.print(millis() - nextSave); Serial.println(" ms late.");
    } else {
      Serial.print("Saving and graphing temps at "); Serial.print(millis() - nextSave); Serial.println(" ms after ideal time.");
    }
    saveTemps();
    tGraph();
    nextSave = nextSave + tSaveInterval;
  }

  if (doTest[RELAY]) {
    flipRelays();
  } else {
    delay(1000);
  }
}
