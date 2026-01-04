/*
 * EEPROM Programmer for 7-Segment Display Lookup Table
 * 
 * This program creates a 4KB lookup table in an EEPROM that maps 8-bit values
 * to 7-segment display patterns in multiple number formats.
 * 
 * HARDWARE CONNECTIONS:
 * - Shift Register (74HC595): SHIFT_DATA=2, SHIFT_CLK=3, SHIFT_LATCH=4
 * - EEPROM Data Bus: D0-D7 on Arduino pins 5-12
 * - EEPROM Write Enable: Pin 13 (active LOW)
 * 
 * EEPROM ADDRESS MAPPING (12 bits = 4096 bytes):
 * A0-A7   : Input value (0-255)
 * A8-A9   : Digit position (0=ones, 1=tens, 2=hundreds, 3=thousands)
 * A10-A11 : Display mode
 *           00 = Unsigned decimal (0-255)
 *           01 = Signed decimal (-128 to +127)
 *           10 = Octal (0-377)
 *           11 = Hexadecimal (0-FF)
 * 
 * 7-SEGMENT BIT MAPPING (assuming standard segment layout):
 * Bit 0: Segment A (top)
 * Bit 1: Segment B (top right)
 * Bit 2: Segment C (bottom right)
 * Bit 3: Segment D (bottom)
 * Bit 4: Segment E (bottom left)
 * Bit 5: Segment F (top left)
 * Bit 6: Segment G (middle)
 * Bit 7: Decimal point (if used)
 */

// Pin definitions
#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

// EEPROM size
#define EEPROM_SIZE 4096

/**
 * Set the address on the EEPROM using shift registers
 * @param address 12-bit address (0-4095)
 * @param outputEnable true to enable EEPROM output (reading), false to disable (writing)
 */
void setAddress(int address, bool outputEnable) {
  // Shift 16-bit address, 8 bits at a time
  // Bit 15 (MSB of first byte) controls output enable (inverted)
  byte firstBits = (address >> 8) | (outputEnable ? 0x00 : 0x80);
  byte lastBits = address & 0xFF;
  
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, firstBits);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, lastBits);

  // Latch the data with one clock pulse
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

/**
 * Read a byte from the EEPROM
 * @param address Address to read from
 * @return Byte value at that address
 */
byte readEEPROM(int address) {
  // Set all data pins as input
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, INPUT);
  }

  setAddress(address, true);  // Enable output

  byte data = 0;
  // Read from D7 to D0 (MSB first)
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

/**
 * Write a byte to the EEPROM
 * @param address Address to write to
 * @param data Byte value to write
 * @param verbose If true, print debug information
 */
void writeEEPROM(int address, byte data, bool verbose) {
  // Print debug log if requested
  if (verbose) {
    // Fixed: Loop 16 times for 16-bit address (was 17)
    for (int i = 15; i >= 0; i--) { 
      Serial.print(bitRead(address, i)); 
    }
    char buffer[10];
    sprintf(buffer, " : %02x", data);    
    Serial.println(buffer);
  } else {
    Serial.print(".");
  }
  
  // Set all data pins as output
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, OUTPUT);
  }

  setAddress(address, false);  // Disable output for writing

  // Write data bits (D0 = LSB, D7 = MSB)
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }

  // Write enable pulse (active LOW)
  digitalWrite(WRITE_ENABLE, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE, HIGH);
  delay(10);  // EEPROM write cycle time (adjust per datasheet)
}

/**
 * Erase the EEPROM by writing 0xFF to all locations
 * @param size Number of bytes to erase
 */
void eraseEEPROM(int size) {
  Serial.print("Erasing EEPROM...");
  for (int address = 0; address < size; address++) {
    writeEEPROM(address, 0xFF, false);
    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" Done!");
}

/**
 * Program an array of bytes into the EEPROM
 * @param data Array of bytes to program
 * @param dataSize Number of bytes in the array
 */
void programBytes(byte data[], int dataSize) {
  Serial.print("Programming EEPROM...");
  for (int address = 0; address < dataSize; address++) {
    writeEEPROM(address, data[address], false);
    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" Done!");
}

/**
 * Print EEPROM contents in hex dump format
 * @param size Number of bytes to display
 */
void printContents(int size) {
  Serial.println("EEPROM Contents:");
  Serial.println("Addr: +0 +1 +2 +3 +4 +5 +6 +7   +8 +9 +A +B +C +D +E +F");
  Serial.println("------------------------------------------------------------");
  
  // Loop through 16-byte rows
  for (int base = 0; base < size; base += 16) {
    byte data[16];

    // Read 16 bytes (or remaining bytes if less than 16)
    int bytesToRead = min(16, size - base);
    for (int offset = 0; offset < bytesToRead; offset++) {
      data[offset] = readEEPROM(base + offset);
    }
    
    // Fill remaining bytes with a placeholder for display
    for (int offset = bytesToRead; offset < 16; offset++) {
      data[offset] = 0;
    }

    // Format and print hex dump
    char buf[80];
    sprintf(buf, "%04x: %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    
    Serial.println(buf);
  }
  Serial.println();
}

/**
 * Verify that written data matches expected values
 * @param data Array of expected byte values
 * @param dataSize Number of bytes to verify
 * @return true if verification passed, false otherwise
 */
bool verifyEEPROM(byte data[], int dataSize) {
  Serial.print("Verifying EEPROM...");
  bool passed = true;
  
  for (int address = 0; address < dataSize; address++) {
    byte readValue = readEEPROM(address);
    if (readValue != data[address]) {
      Serial.println();
      Serial.print("Verification failed at address 0x");
      Serial.print(address, HEX);
      Serial.print(": expected 0x");
      Serial.print(data[address], HEX);
      Serial.print(", got 0x");
      Serial.println(readValue, HEX);
      passed = false;
    }
    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  
  if (passed) {
    Serial.println(" Passed!");
  }
  return passed;
}

/**
 * Main function to write the complete LED display lookup table
 * @param cathode true for common cathode displays, false for common anode
 */
void writeLedEEPROM(bool cathode) {
  bool verbose = false;

  // Write all four display modes
  writeDisplayMode(0, 0, cathode, verbose);  // Unsigned decimal
  writeDisplayMode(0, 1, cathode, verbose);  // Signed decimal
  writeDisplayMode(1, 0, cathode, verbose);  // Octal
  writeDisplayMode(1, 1, cathode, verbose);  // Hexadecimal
}

/**
 * Write a complete display mode to EEPROM
 * @param a11 Address bit 11 (mode selection)
 * @param a10 Address bit 10 (mode selection)
 * @param cathode true for common cathode, false for common anode
 * @param verbose true to print debug output
 */
void writeDisplayMode(bool a11, bool a10, bool cathode, bool verbose) {
  
  // 7-segment patterns for digits 0-9, A-F, blank, and minus sign
  // Pattern uses active-low for common anode (0 = segment on)
  byte digits[] = { 
    0x03, // 0: segments A,B,C,D,E,F
    0x9F, // 1: segments B,C
    0x25, // 2: segments A,B,D,E,G
    0x0D, // 3: segments A,B,C,D,G
    0x99, // 4: segments B,C,F,G
    0x49, // 5: segments A,C,D,F,G
    0x41, // 6: segments A,C,D,E,F,G
    0x1F, // 7: segments A,B,C
    0x01, // 8: all segments
    0x09, // 9: segments A,B,C,D,F,G
    0x11, // A: segments A,B,C,E,F,G
    0xC1, // b: segments C,D,E,F,G
    0x63, // C: segments A,D,E,F
    0x85, // d: segments B,C,D,E,G
    0x61, // E: segments A,D,E,F,G
    0x71, // F: segments A,E,F,G
    0xFF, // blank (all segments off) - used for leading zeros
    0xFD  // minus sign: segment G only
  };

  // Invert patterns for common cathode displays
  if (cathode) {
    for (int i = 0; i < 18; i++) {
      digits[i] = ~digits[i];
    }
  }  
 
  // Digit position encoding for A8-A9
  byte digitPosition[4] = { 0b0000, 0b0001, 0b0010, 0b0011 };
  
  // Mode encoding for A10-A11
  byte mode[4][4] = { 
    { 0b0000, 0b0001, 0b0010, 0b0011 },  // Mode 0: unsigned decimal
    { 0b0100, 0b0101, 0b0110, 0b0111 },  // Mode 1: signed decimal
    { 0b1000, 0b1001, 0b1010, 0b1011 },  // Mode 2: octal
    { 0b1100, 0b1101, 0b1110, 0b1111 }   // Mode 3: hexadecimal
  };

  // Print mode header
  int modeIndex = 2 * a11 + a10;
  Serial.println();
  Serial.println("================================");
  switch (modeIndex) {
    case 0: Serial.println("Programming: Unsigned Decimal"); break;
    case 1: Serial.println("Programming: Signed Decimal"); break;
    case 2: Serial.println("Programming: Octal"); break;
    case 3: Serial.println("Programming: Hexadecimal"); break;
  }
  Serial.println("================================");

  // Determine number base
  int base = 10;  // Default for unsigned and signed decimal
  if (modeIndex == 2) base = 8;   // Octal
  if (modeIndex == 3) base = 16;  // Hexadecimal

  // Determine value range
  int minValue = 0;
  int maxValue = 256;
  if (modeIndex == 1) {  // Signed decimal
    minValue = -128;
    maxValue = 128;
  }
  
  // Program all four digit positions
  for (int digitPos = 0; digitPos < 4; digitPos++) {
    Serial.print("  Digit position ");
    Serial.print(digitPos);
    Serial.print(" ");
    
    // Program all values in the range
    for (int value = minValue; value < maxValue; value++) {
      int displayValue;
      int patternIndex;
      
      // Calculate which digit to display at this position
      int absValue = abs(value);
      int divisor = 1;
      for (int i = 0; i < digitPos; i++) {
        divisor *= base;
      }
      displayValue = (absValue / divisor) % base;
      
      // Suppress leading zeros (replace with blank)
      // Exception: show single "0" in rightmost position when value is zero
      if (displayValue == 0 && absValue < divisor) {
        // Show "0" only in the ones position (digitPos 0) when value is exactly 0
        if (absValue == 0 && digitPos == 0) {
          patternIndex = 0;  // Show "0"
        } else {
          patternIndex = 16;  // Blank pattern
        }
      } else {
        patternIndex = displayValue;
      }
      
      // For signed decimal, show minus sign in leftmost position
      if (modeIndex == 1 && value < 0 && digitPos == 3) {
        patternIndex = 17;  // Minus sign
      }
      
      // Calculate EEPROM address
      // A0-A7: value (need to handle negative numbers for signed mode)
      int addressValue = (value < 0) ? (256 + value) : value;
      int address = addressValue | (mode[modeIndex][digitPos] << 8);
      
      // Write to EEPROM
      writeEEPROM(address, digits[patternIndex], verbose);
    }
    Serial.println();
  }
  
  Serial.println();
}

void setup() {
  // Initialize shift register pins
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  // Initialize write enable pin (active LOW)
  digitalWrite(WRITE_ENABLE, HIGH);  // Disable writes initially
  pinMode(WRITE_ENABLE, OUTPUT);

  // Initialize serial communication
  Serial.begin(57600);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for native USB)
  }
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("EEPROM 7-Segment Display Programmer");
  Serial.println("========================================");
  Serial.println();

  // Uncomment to erase EEPROM before programming
  // eraseEEPROM(EEPROM_SIZE);

  // Program the lookup table
  // Set to 'true' for common cathode displays, 'false' for common anode
  writeLedEEPROM(false);

  // Display programmed contents
  Serial.println();
  Serial.println("Programming complete!");
  Serial.println();
  printContents(EEPROM_SIZE);
}

void loop() {
  // Could add serial command interface here for interactive operation
  // Example commands: read address, write byte, dump range, etc.
}
