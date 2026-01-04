import java.io.*;
import java.nio.file.*;

public class SevenSegment {

	private static int eeprom_size = 8*1024; 		// Default 64kBit (8KB)
	private static String fileName = "out.bin";		// Default out.bin
	private static boolean commonCathode = true;	// Default as common cathode
	private static boolean verbose = true;			// Default as not verbose

	// 7-segment patterns for digits 0-9, A-F, blank, and minus sign
	// Pattern uses active-high for common cathode (1 = segment on)
	// See 5461AS-1 datasheet for reference for controlled segments:
	// 		Bit			8	7	6	5	4	3	2	1
	// 		Segment		DP	G	F	E	D	C	B	A
	//		Pin			3	5	10	1	2	4	7	11
	private static final byte[] digits = {
		(byte) 0x3F, // 0: pattern for digit 0 (0b00111111)
		(byte) 0x06, // 1: pattern for digit 1 (0b00000110)
		(byte) 0x5B, // 2: pattern for digit 2 (0b01011011)
		(byte) 0x4F, // 3: pattern for digit 3 (0b01001111)
		(byte) 0x66, // 4: pattern for digit 4 (0b01100110)
		(byte) 0x6D, // 5: pattern for digit 5 (0b01101101)
		(byte) 0x7D, // 6: pattern for digit 6 (0b01111101)
		(byte) 0x07, // 7: pattern for digit 7 (0b00000111)
		(byte) 0x7F, // 8: pattern for digit 8 (0b01111111)
		(byte) 0x6F, // 9: pattern for digit 9 (0b01101111)
		(byte) 0x77, // A: pattern for letter A (0b01110111)
		(byte) 0x7C, // b: pattern for letter b (0b01111100)
		(byte) 0x39, // C: pattern for letter C (0b00111001)
		(byte) 0x5E, // d: pattern for letter d (0b01011110)
		(byte) 0x79, // E: pattern for letter E (0b01111001)
		(byte) 0x71, // F: pattern for letter F (0b01110001)
		(byte) 0x00, // blank: all segments off (0b00000000)
		(byte) 0x40  // minus: pattern for minus sign (0b01000000)
	};

	public static void main(String[] args) {
		try {

			// Initialize data array with 0x00 (blank)
			byte[] data = new byte[eeprom_size];
			for (int i = 0; i < eeprom_size; i++)
				data[i] = (byte) 0x00;

			// Write all four display modes
			data = writeDisplayMode(data, false, false);  // Unsigned decimal
			data = writeDisplayMode(data, false, true);   // Signed decimal
			data = writeDisplayMode(data, true, false);   // Octal
			data = writeDisplayMode(data, true, true);    // Hexadecimal

			// Write the binary file
			Files.write(Paths.get(fileName), data);

			System.out.println("\nEEPROM file created successfully: " + fileName);
		} catch (IOException e) {
			System.err.println("Error creating bin file: "+ e.getMessage());
			e.printStackTrace();
		}
	}

	/**
	* Write a complete display mode to EEPROM
	* @param data the EEPROM data array
	* @param a11 Address bit 11 (mode selection)
	* @param a10 Address bit 10 (mode selection)
	* @param verbose true to print debug output
	* @return modified data array
	*/
	static byte[] writeDisplayMode(byte[] data, boolean a11, boolean a10) {

		// Mode encoding for A10-A11
		// Digit positions are reversed: [Dig4, Dig3, Dig2, Dig1]
		int modeIndex = (a11 ? 2 : 0) + (a10 ? 1 : 0);
		byte[][] mode = {
			{ 0b0011, 0b0010, 0b0001, 0b0000 },  // Mode 0: unsigned decimal (reversed)
			{ 0b0111, 0b0110, 0b0101, 0b0100 },  // Mode 1: signed decimal (reversed)
			{ 0b1011, 0b1010, 0b1001, 0b1000 },  // Mode 2: octal (reversed)
			{ 0b1111, 0b1110, 0b1101, 0b1100 }   // Mode 3: hexadecimal (reversed)
		};

		// Print mode header
		System.out.print("Generating data array: ");
		switch (modeIndex) {
			case 0: System.out.print("Unsigned Decimal "); break;
			case 1: System.out.print("Signed Decimal   "); break;
			case 2: System.out.print("Octal            "); break;
			case 3: System.out.print("Hexadecimal      "); break;
		}
		if (verbose) System.out.println();

		// Determine number base
		int base = 10;  // Default for unsigned and signed decimal
		if (modeIndex == 2) base = 8;   // Octal
		if (modeIndex == 3) base = 16;  // Hexadecimal

		// Determine value range
		int minValue = 0, maxValue = 256;
		if (modeIndex == 1) {  // Signed decimal
			minValue = -128;
			maxValue = 128;
		}

		// Loop through each value, then generate all digits
		for (int value = minValue; value < maxValue; value++) {

			// Debug output if verbose
			if (verbose) System.out.printf("  Value %4d:", value);

			// Generate all four digit positions for this value
			for (int digitPos = 0; digitPos < 4; digitPos++) {

				// Initialize loop variables
				int displayValue, patternIndex;

				// Calculate which digit to display at this position
				int absValue = Math.abs(value);
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

				// Write to EEPROM with digits value or inverse for common anode
				data[address] = (commonCathode) ? digits[patternIndex] : (byte) ~digits[patternIndex];

				// Debug output if verbose
				if (verbose)
					System.out.printf(" [Dig%d: 0x%04X->0x%02X]",
						digitPos, address, data[address] & 0xFF);

			}


			// Progress indicator for non-verbose mode
			if (!verbose && (value - minValue) %8 == 0) {
				System.out.print(".");
			} else if (verbose) System.out.println();
		}

		if (!verbose) System.out.println(" ✓");
		else System.out.println();
		return data;
	}

}
