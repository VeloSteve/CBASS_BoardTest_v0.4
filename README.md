# CBASS_BoardTest_v0.4
 Exercise hardware components of CBASS-R, board version 0.4
 
 This is a very crude version of what might be called a "unit test".  It simply does something with each hardware section and demonstrates one way of displaying data.  The things tested are
 * TFT display
 * SD card
 * Bluetooth Low Energy
 * Clock
 * Sensor connector
 * Relay connector

Other than the on-board clock, each test requires the appropriate additional hardware to be attached.
Where possible the tests return good or bad, but in many cases the software can't tell what the hardware
is doing, so you much check visually.
