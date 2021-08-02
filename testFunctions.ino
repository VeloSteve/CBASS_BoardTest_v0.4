byte trySD() {
  if (!SDinit()) {
    // One extra try.
    delay(250);
    if (!SDinit()) {
      return false;
    }
  }
  
  logFile = SD.open("TESTLOG.txt", FILE_WRITE);
  logFile.print("Text, no line feed ");
  logFile.println(7);
  logFile.close();
  
  File logFileReading;
  logFileReading = SD.open("TESTLOG.txt", FILE_READ);
  Serial.println("From log file, got:");
  byte b;
    logFileReading.seek(0);
    while (logFileReading.available()) {
    b = logFileReading.read();
    Serial.write(b);
  }
  Serial.println("Done reading SD.  Test passed if you saw \"Text, no line feed 7\"");
  logFileReading.close();
  SD.remove("TESTLOG.txt");
  if (SD.exists("TESTLOG.txt")) {
    Serial.println("Failed to remove TESTLOG.txt");
    return false;
  } else {
    Serial.println("Removed TESTLOG.txt");
    return true;
  }
}
byte SDinit()
{
  Serial.println("In SDinit");
  if (!SD.begin(SD_CS)) {
    Serial.println("SD initialization failed!");
    //init_failed = 1;
    //strcpy(fail_str3, "SD");
    return 0;
  }
  Serial.println("SD initialization done.");
  return 1;
}


byte tftHello() {
  Serial.println("Running tftHello().");
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);  tft.setTextSize(1);
  tft.println("Hello CBASS users!");
  tft.println("");
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.println("This is CBASS-R");
  tft.println("");
  tft.setTextColor(RED);    tft.setTextSize(3);
  tft.println("Less wiring, more science!");
  // If lines run long they wrap, but lines beyond the bottom of 
  // the screen are simply invisible.
  return true;
}

byte checkClock() {
  /* tft
  lcd.clear();
  lcd.print("Clock check");
  lcd.setCursor(0, 1);
   */
  while (!rtc.begin()) {
    Serial.println("Couldn't start RTC");
    delay(2000);
  }
  Serial.println("Started RTC");

  while (!rtc.isrunning()) {
    Serial.println("Failed to communicate with RTC.  Is one connected?");
    // XXX this shouldn't work, but see if it changes anything:
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, 0, 0, uploadLag));

    delay(4000);
  }
  Serial.println("RTC is running.");
  showTime(true, true);
  DateTime now = rtc.now();
  if (now.year() < 2021) {
    Serial.println("WARNING: clock has not been set or battery is missing.");
    delay(250);
    return false;
  }
  delay(1000);
  return true;
}


const char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char monthsOfTheYear[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**
 * Show current time on the serial monitor and/or LCD.  For the LCD assume the
 * cursor is already set as required.
 */

void showTime(boolean onSerial, boolean onLCD) {
  if (!onSerial && !onLCD) return;
  DateTime now = rtc.now();

  if (onSerial) {
    Serial.print(now.hour());
    Serial.print(':');
    if (now.minute() < 10) Serial.print("0");
    Serial.print(now.minute());
    Serial.print(':');
    if (now.second() < 10) Serial.print("0");
    Serial.print(now.second());
    Serial.print("   ");

    // Check: are days zero-based?
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(", ");
    Serial.print(now.day());
    Serial.print('/');
    Serial.print(now.month());
    Serial.print('/');
    Serial.println(now.year());
  }
  if (onLCD) {
    // Expected example:
    // 10:09:12   Mon, 13/11/2021
    // This is 26 characters.  Try to fit one LCD line:
    // 10:09:12 Mon 13/11/2021 (23)
    // 10:09:12 13 Nov (15) <- use this
    /*tft
    lcd.print(now.hour());
    lcd.print(':');
    if (now.minute() < 10) lcd.print("0");
    lcd.print(now.minute());
    lcd.print(':');
    if (now.second() < 10) lcd.print("0");
    lcd.print(now.second());
    lcd.print(" ");
  
    lcd.print(now.day());
    lcd.print(' ');
    lcd.print(monthsOfTheYear[now.month()-1]);
    */
  }
}

void setRelayPins() {
  for (int i=0; i<4; i++) {
    pinMode(ChillRelay[i], OUTPUT);
    pinMode(HeaterRelay[i], OUTPUT);
    pinMode(LightRelay[i], OUTPUT);
  }
}

void switchRelay(byte function, byte i, byte state) {
  if (function == CHILL) {
    digitalWrite(ChillRelay[i], state);
  } else if (function == HEAT) {
    digitalWrite(HeaterRelay[i], state);
  } else if (function == LIGHT) {
    digitalWrite(LightRelay[i], state);
  } else {
    Serial.println("ERROR - relay function does not exist!");
  }
  relayState[function][i] = state;
  showTankLabels(i);
}

void flipRelays() {
  int i;
  word onGap = 350;
  word offGap = 200;
  word allOn = 400;
  Serial.println("Flipping 4 chiller relays on.");
  for (i=0; i<4; i++) {
    switchRelay(CHILL, i, RELAY_ON);
    delay(onGap);
  }
  delay(400);
  Serial.println("Flipping 4 heater relays on.");
  for (i=0; i<4; i++) {
    switchRelay(HEAT, i, RELAY_ON);
    delay(onGap);
  }
  delay(400);
  Serial.println("Flipping 4 light relays on."); 
  for (i=0; i<4; i++) {
    switchRelay(LIGHT, i, RELAY_ON);
    delay(onGap);
  }
  delay(allOn);
  Serial.println("Flipping 12 relays off.");
  
    for (i=0; i<4; i++) {
    switchRelay(CHILL, i, RELAY_OFF);
    
    delay(offGap);
  }
  for (i=0; i<4; i++) {
    switchRelay(HEAT, i, RELAY_OFF);

    delay(offGap);
  }
  for (i=0; i<4; i++) {
    switchRelay(LIGHT, i, RELAY_OFF);

    delay(offGap);
  }
}





 void CharacterGrid()
 {
  int row = 16;
  int col = 16;
  int left, top;   
  int l = 10;
  int t = 0;
  int w = 19;
  int h = 15;
  int hgap = 0;
  int vgap = 0;
  int id = 0;

  for(int j = 0; j < row; j++){
    for (int i = 0; i < col; i++){
       left = l + i*(w + vgap);
       top = t + j*(h + hgap);
       tft.drawChar(left, top, id, WHITE, BLUE, 2);
       id++;
      }
   }
 }
