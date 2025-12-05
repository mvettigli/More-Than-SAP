# More-Than-SAP

The projects starts from Ben Eater's SAP-1 build and improves memory availability and CPU functions.

Specifically it implements:
- 16-bit address bus addressable to data bus
- 16-bit stack pointer
- 16 bit program counter
- RAM 64 kByte using CY7C199
- ROM 32 kByte using AT28C256
- 8 bit instruction register
- 2 8-bit generic purpose registers
- 2 8-bit up/down counter registers with clear
- 2 8-bit shift registers with clear
- ALU with arithmetic (ADD, SUB) and logic functions (AND, OR, XOR)
- Register file of 4 units that can be addressed to ALU from 6 available
- 4-bit flag register: zero, sign, carry and overflow
