/*** Address (15 bits)


[I7, I6, I5, I4, I3, I2, I1, I0] = Instruction
					[S2, S1, S0] = Step Sequencer
				[F3, F2, F1, F0] = Flag status
*/

/** Control bits (32 bits)
 ** ----------------------

[___, OC2, OC1, OC0] = ALU operator
[XC3, XC2, XC1, XC0] = ALU operand
[DC3, DC2, DC1, DC0] = Data bus out
[___, ___, AC1, AC0] = Address bus out
[BC3, BC2, BC1, BC0] = Data bus in
[MAR, MCE, SC1, SC0] = Stack pointer and MAR
[___, HLT, PCR, PCE] = Program counter and halt
[___, ___, FLS, SSR] = Step sequencer and flag
*/

public class IsaGenerator {

// Common
private static final int NOP = 0b0000_0000_0000_0000_0000_0000_0000_0000;	//ASY


// ALU operators
private static final int ADD = 0b0000_0000_0000_0000_0000_0000_0000_0000;	//ASY
private static final int SUB = 0b0001_0000_0000_0000_0000_0000_0000_0000;	//ASY
private static final int AND = 0b0010_0000_0000_0000_0000_0000_0000_0000;	//ASY
private static final int OR  = 0b0011_0000_0000_0000_0000_0000_0000_0000;	//ASY
private static final int XOR = 0b0100_0000_0000_0000_0000_0000_0000_0000;	//ASY

// ALU operands
private static final int AA = 0b0000_0000_0000_0000_0000_0000_0000_0000;	//ASY
private static final int AB = 0b0000_0001_0000_0000_0000_0000_0000_0000;	//ASY
private static final int AC = 0b0000_0010_0000_0000_0000_0000_0000_0000;	//ASY
private static final int AD = 0b0000_0011_0000_0000_0000_0000_0000_0000;	//ASY
private static final int BA = 0b0000_0100_0000_0000_0000_0000_0000_0000;	//ASY
private static final int BB = 0b0000_0101_0000_0000_0000_0000_0000_0000;	//ASY
private static final int BC = 0b0000_0110_0000_0000_0000_0000_0000_0000;	//ASY
private static final int BD = 0b0000_0111_0000_0000_0000_0000_0000_0000;	//ASY
private static final int CA = 0b0000_1000_0000_0000_0000_0000_0000_0000;	//ASY
private static final int CB = 0b0000_1001_0000_0000_0000_0000_0000_0000;	//ASY
private static final int CC = 0b0000_1010_0000_0000_0000_0000_0000_0000;	//ASY
private static final int CD = 0b0000_1011_0000_0000_0000_0000_0000_0000;	//ASY
private static final int DA = 0b0000_1100_0000_0000_0000_0000_0000_0000;	//ASY
private static final int DB = 0b0000_1101_0000_0000_0000_0000_0000_0000;	//ASY
private static final int DC = 0b0000_1110_0000_0000_0000_0000_0000_0000;	//ASY
private static final int DD = 0b0000_1111_0000_0000_0000_0000_0000_0000;	//ASY

// Data bus out
private static final int ARO = 0b0000_0000_0001_0000_0000_0000_0000_0000;	//ASY
private static final int BRO = 0b0000_0000_0010_0000_0000_0000_0000_0000;	//ASY
private static final int CRO = 0b0000_0000_0011_0000_0000_0000_0000_0000;	//ASY
private static final int DRO = 0b0000_0000_0100_0000_0000_0000_0000_0000;	//ASY
private static final int RAO = 0b0000_0000_0101_0000_0000_0000_0000_0000;	//ASY
private static final int ROO = 0b0000_0000_0110_0000_0000_0000_0000_0000;	//ASY
private static final int INO = 0b0000_0000_0111_0000_0000_0000_0000_0000;	//ASY
private static final int FRO = 0b0000_0000_1000_0000_0000_0000_0000_0000;	//ASY
private static final int BLO = 0b0000_0000_1001_0000_0000_0000_0000_0000;	//ASY
private static final int BMO = 0b0000_0000_1010_0000_0000_0000_0000_0000;	//ASY
private static final int ALO = 0b0000_0000_1011_0000_0000_0000_0000_0000;	//ASY
private static final int MIO = 0b0000_0000_1100_0000_0000_0000_0000_0000;	//ASY

// Address bus out
private static final int PCO = 0b0000_0000_0000_0001_0000_0000_0000_0000;	//ASY
private static final int MAO = 0b0000_0000_0000_0010_0000_0000_0000_0000;	//ASY
private static final int SPO = 0b0000_0000_0000_0011_0000_0000_0000_0000;	//ASY

// Data bus in
private static final int ARI = 0b0000_0000_0000_0000_0001_0000_0000_0000;	//CLK
private static final int BRI = 0b0000_0000_0000_0000_0010_0000_0000_0000;	//CLK
private static final int CRI = 0b0000_0000_0000_0000_0011_0000_0000_0000;	//CLK
private static final int DRI = 0b0000_0000_0000_0000_0100_0000_0000_0000;	//CLK
private static final int RAI = 0b0000_0000_0000_0000_0101_0000_0000_0000;	//CLK
private static final int ORI = 0b0000_0000_0000_0000_0110_0000_0000_0000;	//CLK
private static final int INI = 0b0000_0000_0000_0000_0111_0000_0000_0000;	//CLK
private static final int FRI = 0b0000_0000_0000_0000_1000_0000_0000_0000;	//CLK
private static final int MLI = 0b0000_0000_0000_0000_1001_0000_0000_0000;	//CLK
private static final int MMI = 0b0000_0000_0000_0000_1010_0000_0000_0000;	//CLK
private static final int PLI = 0b0000_0000_0000_0000_1011_0000_0000_0000;	//CLK
private static final int PMI = 0b0000_0000_0000_0000_1100_0000_0000_0000;	//CLK

// Stack pointer
private static final int SIN = 0b0000_0000_0000_0000_0000_0001_0000_0000;	//CLK
private static final int SDE = 0b0000_0000_0000_0000_0000_0010_0000_0000;	//CLK
private static final int SCL = 0b0000_0000_0000_0000_0000_0011_0000_0000;	//CLK

// Memory address register
private static final int MCE = 0b0000_0000_0000_0000_0000_0100_0000_0000;	//CLK
private static final int MAR = 0b0000_0000_0000_0000_0000_1000_0000_0000;	//ASY

// Program counter & halt
private static final int PCE = 0b0000_0000_0000_0000_0000_0000_0001_0000;	//~CLK
private static final int PCR = 0b0000_0000_0000_0000_0000_0000_0010_0000;	//ASY
private static final int HLT = 0b0000_0000_0000_0000_0000_0000_0100_0000;	//ASY

// Step sequencer & flag selector
private static final int SSR = 0b0000_0000_0000_0000_0000_0000_0000_0001;	//ASY
private static final int FLS = 0b0000_0000_0000_0000_0000_0000_0000_0010;	//ASY


private final int size = 1 << 15;  // 2^15 = 32768
private int[] controls = new int[size];

public void main(String args[]) {

    // TODO: P1 SSR to be ~CLK to avoid one clock for ending all instruction
    // TODO: P2 MAR to be ~CLK instead of ASY to gain one clock for zero page
    // TODO: P5 change label to program counter ~CLK instead of CLK
    // TODO: P6 ADD/SUB with carry bit from flag register

    encode(0b0000_0000, NOP);		               // 000: NOP instruction

    encode(0b0000_0001, PCO | MIO | ARI);          // 001: LDA #00 immediate
    encode(0b0000_0010, MAR,                       // 002: LDA $00 zero page
                        PCO | MIO | MLI | PCE,
                        MAO | MIO | ARI);
    encode(0b0000_0011, PCO | MIO | MLI | PCE,     // 003: LDA $0000 absolute
                        PCO | MIO | MMI | PCE,
                        MAO | MIO | ARI);

    encode(0b0000_0100, PCO | MIO | BRI);          // 004: LDB #00 immediate
    encode(0b0000_0101, MAR,                       // 005: LDB $00 zero page
                        PCO | MIO | MLI | PCE,
                        MAO | MIO | BRI);
    encode(0b0000_0110, PCO | MIO | MLI | PCE,     // 006: LDB $0000 absolute
                        PCO | MIO | MMI | PCE,
                        MAO | MIO | BRI);

    encode(0b0000_0111, PCO | MIO | CRI);          // 007: LDC #00 immediate
    encode(0b0000_1000, MAR,                       // 008: LDC $00 zero page
                        PCO | MIO | MLI | PCE,
                        MAO | MIO | CRI);
    encode(0b0000_1001, PCO | MIO | MLI | PCE,     // 009: LDC $0000 absolute
                        PCO | MIO | MMI | PCE,
                        MAO | MIO | CRI);

    encode(0b0000_1010, PCO | MIO | DRI);          // 010: LDD #00 immediate
    encode(0b0000_1011, MAR,                       // 011: LDD $00 zero page
                        PCO | MIO | MLI | PCE,
                        MAO | MIO | DRI);
    encode(0b0000_1100, PCO | MIO | MLI | PCE,     // 012: LDD $0000 absolute
                        PCO | MIO | MMI | PCE,
                        MAO | MIO | DRI);

    encode(0b0000_1101, ARO | BRI);                // 013: MOV A to B
    encode(0b0000_1110, ARO | CRI);                // 014: MOV A to C
    encode(0b0000_1111, ARO | DRI);                // 015: MOV A to D

    encode(0b0001_0000, BRO | ARI);                // 016: MOV B to A
    encode(0b0001_0001, BRO | CRI);                // 017: MOV B to C
    encode(0b0001_0010, BRO | DRI);                // 018: MOV B to D

    encode(0b0001_0011, CRO | ARI);                // 019: MOV C to A
    encode(0b0001_0100, CRO | BRI);                // 020: MOV C to B
    encode(0b0001_0101, CRO | DRI);                // 021: MOV C to D

    encode(0b0001_0110, DRO | ARI);                // 022: MOV D to A
    encode(0b0001_0111, DRO | BRI);                // 023: MOV D to B
    encode(0b0001_1000, DRO | CRI);                // 024: MOV D to C


    printControls();

}

private void encodeInstructionFlag(int instruction, int flags, int... ctrls)
	throws IllegalArgumentException {

    if (instruction >= 256) throw new IllegalArgumentException();
    if (ctrls.length > 6) throw new IllegalArgumentException();

    // set fetch microcode
    controls[instruction << 7 | 0b0000 << 4 | flags ] = PCO | MIO | INI | PCE;

    for (int step = 1; step < 8; step++) {
        int address = instruction << 7 | step << 4 | flags;
        int stepCase = Integer.signum(ctrls.length +1 - step);
        switch (stepCase) {
            case 1:
                controls[address] = ctrls[step-1];
                break;
            case 0:
                controls[address] = SSR;
                break;
            case -1:
                controls[address] = NOP;
                break;
        }
    }
}

private void encode(int instruction, int... ctrls) {
    // loop through all flag combination
    for (int flags = 0; flags < 16; flags++)
        encodeInstructionFlag(instruction, flags, ctrls);
}

private void printControls() {
    System.out.println("Address (binary) : Control Bits (binary)");
    System.out.println("=".repeat(80));

    for (int i = 0; i < size; i++) {
        if (controls[i] != 0) {  // Only print non-zero entries
            String address = String.format("%15s", Integer.toBinaryString(i)).replace(' ', '0');
            String control = String.format("%32s", Integer.toBinaryString(controls[i])).replace(' ', '0');

            // Format with underscores for readability
            String formattedAddress = formatBinary(address, 8, 3, 4);
            String formattedControl = formatBinary(control, 4, 4, 4, 4, 4, 4, 4, 4);

            System.out.println(formattedAddress + " : " + formattedControl);
        }
    }
}
private String formatBinary(String binary, int... groupSizes) {
    StringBuilder formatted = new StringBuilder();
    int position = 0;
    int groupIndex = 0;

    for (int i = 0; i < binary.length(); i++) {
        // Add underscore if we've completed a group (but not at the start)
        if (position > 0 && groupIndex < groupSizes.length && position == groupSizes[groupIndex]) {
            formatted.append('_');
            groupIndex++;
            position = 0;
        }
        formatted.append(binary.charAt(i));
        position++;
    }
    return formatted.toString();
}

}
