// Display
#define TFT_CS   4
#define TFT_DC   3
// Temperature sensor
#define TempSensorPin 8
// Relay pins, as used in version 0.4
const int ChillRelay[] = {23, 26, 25, 18};  // green, purple/brown, blue, white
const int HeaterRelay[] = {27, 22, 19, 24}; // blue, green, yellow, orange
const int LightRelay[] = {17, 16, 15, 14};  // purple/brown, white, yellow, orange

// SD card.  Note that there is a second SD card on the back of the display, which we don't currently use.
#define SD_DETECT  10
#define SD_CS  11

// ----  Constants ----
// Parameters for the display in use.  Note that colors are on a 16-bit scale, not the more common 24.
#define DARKGREEN 0x05ED  // Normal green is too pale on white.  127->31 is too dark.  192->47 is better.
#define LIGHTBLUE 0x1F3A  // Normal blue is too dark on black. 31, 31, 255 in "normal" rgb is 3F56 here.
// Easier names for some of the colors in the ILI9341 header file.
#define BLACK   ILI9341_BLACK
#define BLUE    ILI9341_BLUE
#define RED     ILI9341_RED
#define GREEN   ILI9341_GREEN
#define CYAN    ILI9341_CYAN
#define MAGENTA ILI9341_MAGENTA
#define YELLOW  ILI9341_YELLOW
#define WHITE   ILI9341_WHITE

#define TFT_WIDTH 320
#define TFT_HEIGHT 240
#define TFT_LAND 3  // Rotation for normal text  1 = SPI pins at top, 3 = SPI at bottom.
#define TFT_PORT 2  // Rotation for y axis label, typically 1 less than TFT_LAND
