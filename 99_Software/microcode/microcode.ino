#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

void setAddress(int address, bool outputEnable) {
  // Shift 16-bit address, 8 bits at a time
  byte firstBits = (address >> 8) | (outputEnable ? 0x00 : 0x80);
  byte lastBits = address;
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, firstBits);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, lastBits);

  // Latch the data with one clock pulse
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

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

void writeEEPROM(int address, byte data) {
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

  // Write enable pulse
  digitalWrite(WRITE_ENABLE, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE, HIGH);
  delay(10);  // Write cycle time
}

void eraseEEPROM(int size) {
  Serial.print("Erasing EEPROM...");
  for (int address = 0; address < size; address++) {
    writeEEPROM(address, 0xff);
    if (address % 64 == 0) {
      Serial.print(".");  // Progress indicator
    }
  }
  Serial.println(" Done!");
}

// Fixed function signature and logic
void programBytes(byte data[], int dataSize) {
  Serial.print("Programming EEPROM...");
  for (int address = 0; address < dataSize; address++) {
    writeEEPROM(address, data[address]);
    if (address % 64 == 0) {
      Serial.print(".");  // Progress indicator
    }
  }
  Serial.println(" Done!");
}

void printContents(int size) {
  Serial.println("EEPROM Contents:");
  // Loop through 16-byte rows
  for (int base = 0; base < size; base += 16) {
    byte data[16];

    // Read 16 bytes (or remaining bytes if less than 16)
    int bytesToRead = min(16, size - base);
    for (int offset = 0; offset < bytesToRead; offset++) {
      data[offset] = readEEPROM(base + offset);
    }

    // Format and print hex dump
    char buf[80];
    sprintf(buf, "%04x: %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    
    Serial.println(buf);
 
  }

}

// Utility function to verify written data
bool verifyEEPROM(byte data[], int dataSize) {
  Serial.print("Verifying EEPROM...");
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
      return false;
    }
    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" Passed!");
  return true;
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
  Serial.println("EEPROM Programmer Ready");
  Serial.println("========================");

  // Erase the EEPROM
  //eraseEEPROM(2048);

  // Display current contents
  printContents(2048);

  // Example: Program some test data
  /*
    byte testData[] = {0xAA, 0x55, 0xFF, 0x00, 0x12, 0x34, 0x56, 0x78};
    int testSize = sizeof(testData);

    programBytes(testData, testSize);
    verifyEEPROM(testData, testSize);
    printContents(16);  // Show first 16 bytes
  */
}

// 1** Alu Operation ---
// 00: OC0 = Alu code 0
// 01: OC1 = Alu code 1
// 02: OC2 = Alu code 2
// 03: OC3 = Alu code 3
// --- Register file ---
// 04: XC0 = X code 0
// 05: XC1 = X code 1
// 06: YC0 = Y code 0
// 07: YC1 = Y code 1
// 2** Data bus out code ---
// 08: DC0 = Data bus out code 0
// 09: DC1 = Data bus out code 1
// 10: DC2 = Data bus out code 2
// 11: DC3 = Data bus out code 3
// --- Address out code ---
// 12: AC0 = Address out code 0
// 13: AC1 = Address out code 1
// 14: AC2 = Address out code 2
// 15: AC3 = Address out code 3
// 3** Shift registers ---
// 16: LC0 = Shift register L code 0
// 17: LC1 = Shift register L code 1
// 18: RC0 = Shift register L code 0
// 19: RC1 = Shift register L code 1
// --- Count registers ---
// 20: IC0 = Count register I code 0
// 21: IC1 = Count register I code 1
// 22: JC0 = Count register J code 0
// 23: JC1 = Count register J code 1
// 4** Data in code ---
// 24: BC0 = Data bus in code 0
// 25: BC1 = Data bus in code 1
// 26: BC2 = Data bus in code 2
// 27: BC3 = Data bus in code 3
// --- Stack Pointer & MAR ---
// 28: SC0 = Stack pointer code 0
// 29: SC1 = Stack pointer code 1
// 30: MC0 = MAR code 0
// 31: MC1 = MAR code 1
// 5** Others ---
// 32: PCE = Program Counter Enable (0 = disable, 1 = enable)
// 33: FLS = Flag Register Select (0 = ALU, 1 = Bus)
// 34: PCR = Program Counter Reset (1 = disable, 0 = enable)
// 35: SSR = Step Sequencer Reset (1 = disable, 0 = enable)
// 36: HLT = Halt (1 = disable, 0 = enable)
// 37: RST = Reset (1 = disable, 0 = enable)
// 38: Unused
// 39: Unused

#define ADD 0b_00000000_00000000_00000000_00000000_00000000_00000000
#define SUB 0b_00000001_00000000_00000000_00000000_00000000_00000000
#define AND 0b_00000010_00000000_00000000_00000000_00000000_00000000
#define OR  0b_00000011_00000000_00000000_00000000_00000000_00000000
#define XOR 0b_00000100_00000000_00000000_00000000_00000000_00000000



void loop() {
  // Main loop - could add serial command interface here
}
