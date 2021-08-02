byte bleSetup() {

  // ===== BLE setup stuff =====
  if ( !ble.begin(VERBOSE_MODE) )
  {
    // Problem: begin returns true even if there is no BLE module!
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );
  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      ble.info();
      error(F("Couldn't factory reset.  Is firmware at 0.8.1?"));
    }
  }
  /* Disable command echo from Bluefruit */
  ble.echo(false);

  // Try to set a custom advertised name
  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=MyLab CBASS-R 1")) ) {
    error(F("Could not set device name?"));
  }

  // We don't currently use the LED, but setting a value serves
  // as a test for updated firmware.
  if (! ble.sendCommandCheckOK(F("AT+HWMODELED=DISABLE")) ) {
    error(F("Could not set LED mode.  Is the firmware updated?"));
  }

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();


  Serial.println(F("Please use Adafruit Bluefruit LE or Serial Bluetooth Terminal app to connect in UART mode"));
  Serial.println(F("d for a debug response"));
  Serial.println(F("t to start temperature updates"));
  Serial.println();
  ble.verbose(false);  // debug info is a little annoying after this point!
  return true;

}

// Check for and respond to BLE inputs with actions, data, or both.  This is only
// called if there is already a connection.
void interact() {


    // Check for incoming characters from Bluefruit
    ble.println(F("AT+BLEUARTRX"));
    ble.readline();
    if (strcmp(ble.buffer, "OK") == 0) {
      // no data
      delay(bleDelay);
      return;
    }
    ble.waitForOK();
    // Some data was found, its in the buffer
    Serial.print(F("[Recv] ")); Serial.println(ble.buffer);
    handleRequest();

}

void tx() {
    ble.print(F("AT+BLEUARTTX="));
}
/**
 * Each set of prints must end with exactly one println and a waitForOK.
 * Encapsulate that here for consistency.  Don't use ble.println anywhere else.
 */
void endtx() {
  ble.println();
  // check response stastus
  if (! ble.waitForOK() ) {
    Serial.println(F("Failed to send?"));
  }
}

void handleRequest() {
  // Note: don't do NVM read stuff here - it could wipe the buffer.
  // First, just acknowledge.
  Serial.println(F("In handleRequest"));


  /*
  tx();
  ble.print(F("\\nThanks.  Got "));
  ble.print(ble.buffer);
  ble.print("\\n");
  endtx();
   */
  // LESSON LEARNED: more than one println without a matching AT_BLEUARTTX can mess up later functions!



  // Now act on the message
  // Each "if" with any printing to ble MUST have exactly one AT command, one println (after any prints) and one waitForOK!
  // This is now satisfied by starting with tx() and ending with endtx(), bracketing any ble.print lines.
  // Note that character 48 is decimal integer 0.


  char c = ble.buffer[0];

  if (c == 'D' || c == 'd') {
    // Show full schedule.
    Serial.println(F("Hello BLE!"));
    tx();
    ble.print(F("Hello BLE!\\n"));
    endtx();
  } else if (c == 'T' || c == 't') {
    // Run on demand.
    tx();
    ble.print(F("Code not ready.\\n"));
    endtx();

  } else if (c == 'h' || c == 'H') {
    // Help message.
    tx();
    ble.print(F("Send\\n d for Hello BLE\\n t for temperatures\\n h for this message\\n"));
    endtx();
  }  else {
    // Warn of invalid instruction.
    tx();
    ble.print(c);
    ble.print(F(" is not a valid instruction.  Try h.\\n"));
    endtx();
  }
  
}
