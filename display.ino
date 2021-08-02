/* This file will contain functions for updating the TFT LCD display. */
#define LABELROW 0
#define TARGETROW 1
#define GRAPHROW 3 
// Number of pixels per line/row in the font size used (2).  Characters are 14, so allow one pixel between lines. 
// We also assume that characters are square, so this is also uses as their width.
#define LINEHEIGHT 15 

boolean firstGraphPass = 1;


void startDisplay() {
    tft.begin();
    tft.setRotation(TFT_LAND); 
    tft.fillScreen(BLACK);
}

/**
 * Draw labels for all 4 tanks.
 */
void showTankLabels() {
  for (int i = 0; i < 4; i++) {
    showTankLabels(i);
  }
}

/**
   Place the label for tank i in row LABELROW, normally at the top of the screen.
*/
void showTankLabels(int i) {
  tft.setTextColor(WHITE);  tft.setTextSize(2);
  word color;
  word start;  // x location of the label
  // Parts of the label relative to start.
  word box1 = 0, text = box1 + 1.5*LINEHEIGHT, box2 = text + 1.5*LINEHEIGHT;
  word y = LABELROW * LINEHEIGHT; // typically zero

  start = i * TFT_WIDTH / 4;
  // Red box for hot, blue for cool, yellow for both on, black for neither.
  if (relayState[HEAT][i] && relayState[CHILL][i]) {
    color = YELLOW;
  } else if (relayState[HEAT][i]) {
    color = RED;
  } else if (relayState[CHILL][i]) {
    color = BLUE;
  } else {
    color = BLACK;
  }
  tft.fillRect(start + box1, y, LINEHEIGHT, LINEHEIGHT, color);
  // Text, A-D
  tft.setCursor(start + text, y);
  tft.print(tankNames[i]);
  // White box for light on, black for off.
  if (relayState[LIGHT][i]) {
    color = WHITE;
  } else {
    color = BLACK;
  }
  tft.fillRect(start + box2, y, LINEHEIGHT, LINEHEIGHT, color);
}

/**
 * For each tank, write the target temperature in TARGETROW of the screen and the
 * current temperature below it.  Targets are in white, but actual temps are blue
 * if cold and red if hot.
 * This uses 2 lines of text.
 */
void showTemps() {
  double fakeTarget = 24.0;  // XXX in real code, use the current target temperatures.
  double warnDegrees = 0.15;

  // Display temperatures as text.
  showTankLabels();  // Maybe move this out?
  // Blank just the temperature text area, otherwise the display blinks unnecessarily.
  word y = TARGETROW * LINEHEIGHT; 
  tft.fillRect(0, y, TFT_WIDTH, LINEHEIGHT*2, BLACK);
  tft.setTextColor(WHITE);  tft.setTextSize(2);
  word start;
  for (byte i = 0; i<4; i++) {
    start = LINEHEIGHT/2 + i * TFT_WIDTH / 4;  // 1/4 of the screen with, plus half a character.
    // Target
    tft.setCursor(start, y);
    tft.setTextColor(WHITE);
    tft.print(fakeTarget, 1); tft.print(deg_tft);
    if (i == 3) tft.print("C");
    // Current
    tft.setCursor(start, y + LINEHEIGHT);
    if (TempInput[i] < fakeTarget - warnDegrees) {
      tft.setTextColor(LIGHTBLUE);
    } else if (TempInput[i] > fakeTarget + warnDegrees) {
      tft.setTextColor(RED);
    } else {
      tft.setTextColor(WHITE);
    }
    tft.print(TempInput[i], 1); tft.print(deg_tft);
    if (i == 3) tft.print("C");
  }
  return;
}


void tGraph() {
  unsigned long graphTic = millis();
  word top = LINEHEIGHT*GRAPHROW; // top of graph area relative to 0 at top of screen
  word maxH = TFT_HEIGHT-1;
  word maxW = TFT_WIDTH-1;
  tft.setTextColor(WHITE);

  if (firstGraphPass) {
    // Draw y label vertically.
    tft.setRotation(TFT_PORT);
    tft.setCursor(30, 0);
    tft.print("  Temp "); tft.print(deg_tft); tft.print("C");
  
    // Axes and x label drawn in landscape mode.
    tft.setRotation(TFT_LAND);
    tft.setCursor(80, maxH - LINEHEIGHT);
    tft.print("  Time, "); tft.print("sec");
    tft.drawLine(LINEHEIGHT, maxH - LINEHEIGHT, LINEHEIGHT, top, WHITE);
    tft.drawLine(LINEHEIGHT, maxH - LINEHEIGHT, maxW, maxH - LINEHEIGHT, WHITE);
  } 
  firstGraphPass = 0;
  // Clear inside the axes before drawing new lines.
  tft.fillRect(LINEHEIGHT+1, top, maxW - LINEHEIGHT, maxH - top - LINEHEIGHT-1, BLACK);
  


  word maxPts = 60;  // Something in the range of 100 or 200 pts?
  word bytesPerLine = 31; // Lines take roughly this much space. 26 gives 8 lines for 10.  30 starts at 10 but drops to 9.
  // with 20-point target, 32 gives 21 lines (after the initial point).
  File logFile = SD.open("GRAPHPTS.TXT", FILE_READ);

  if (!logFile) {
    Serial.println("ERROR: failed to open temperature file GRAPHPTS.TXT.");
    return;
  }
  // We only plot about the last maxPts values, so skip the beginning of the 
  // file if it is longer than that.
  if (logFile.size() > maxPts * bytesPerLine) {
    logFile.seek(logFile.size() - maxPts * bytesPerLine);
    // Read until we have consumed one linefeed.
    while (logFile.read() != '\n') {
      //Serial.print("Skip");
    }
  }
  // Now we should be at the beginning of a line, whether at the start of the file or not.
  unsigned long msTime;
  float temps[4];
  readOneTemperatureLine(logFile, &msTime, temps);
  // Scale and draw.
  // tSaveInterval is the minimum milliseconds between new temperature lines.
  // x range in minutes and pixels
  float timeMin = (float)msTime/60000.;
  float timeRange = 1.05 * ((float)tSaveInterval * (float)maxPts) / 60000.;  // Times 1.05 to allow for uneven sampling without plotting beyond the screen.
  word xMin = LINEHEIGHT+1;
  word xRange = maxW - xMin;
  // y range in degrees and pixels
  float tempMin = 15;
  float tempMax = 35;
  float tempRange = tempMax - tempMin;
  word yMin = maxH - LINEHEIGHT - 1; // Due to graphics coordinates maximum y (points) is at minimum temperature.
  word yRange = yMin - top;
  
  // Now deal with the current (first) point.
  word x = xMin + ((float)msTime/60000. - timeMin)/timeRange * xRange;
  word y = yMin - (temps[0]-tempMin)/tempRange * yRange;
  tft.drawPixel(x, y, WHITE);
  word oldX = x;
  word oldY = y;
  // Read the rest of the file, drawing lines between points.
  word lineCount = 0; // Start from zero, because the line read above is only a starting point.
  while (logFile.available()) {
    if (readOneTemperatureLine(logFile, &msTime, temps)) {
      x = xMin + ((float)msTime/60000. - timeMin)/timeRange * xRange;
      y = yMin - (temps[0]-tempMin)/tempRange * yRange;
  
      tft.drawLine(oldX, oldY, x, y, WHITE);
      if (x >= TFT_WIDTH) {
        Serial.print("==== WARNING: plotting beyond screen. x = "); Serial.print(x); 
            Serial.print(" >= "); Serial.print(TFT_WIDTH); Serial.println(" ====");
      }
      oldX = x;
      oldY = y;
      lineCount++;
    }
  }
  logFile.close();
  Serial.print("Plotted lines, n = "); Serial.print(lineCount); Serial.print(" in ");
      Serial.print(millis() - graphTic); Serial.println(" milliseconds");
}

boolean readOneTemperatureLine(File logFile, unsigned long *t, float temps[4]) {
  char buff[12];  // For characters in the current value.
  char c;
  byte i = 0;
  // Get milliseconds since start.
  c = logFile.read();
  if (c < 0) return 0;
  while (c != ',' && c != -1) {
    buff[i++] = c; 
    c = logFile.read();
  }

  buff[i] = '\0';
  *t = atol(buff);
  delay(3);
  // Now read 4 temperatures.
  for (byte j=0; j<4; j++) {
    i = 0;
    c = logFile.read();
    while (c != ','  && c != '\n'  && c != -1) {
      buff[i++] = c; 
      c = logFile.read();
    }
    buff[i] = '\0';
    temps[j] = atof(buff);
    if (temps[j] == 0.0) return 0;
    //Serial.print("T[");Serial.print(j);Serial.print("] = "); Serial.println(temps[j]);
  }

  return 1;
}
