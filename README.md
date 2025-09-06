
# Virtual Machine

This project is a two-stage compiler that translates a custom virtual machine bytecode into MIPS assembly code.

## Architecture and Flow

The VM operates in a clear, multi-step pipeline. It takes a custom bytecode file as input, processes it through a frontend and backend, and produces a standard MIPS assembly file as output.

**`Input .vbm code`** ➡️ **`Parser (Frontend)`** ➡️ **`Intermediate Representation (IR)`** ➡️ **`MIPS Generator (Backend)`** ➡️ **`Output .s File`**

1.  **Input File (`.vbm format`)**: A text file containing hexadecimal strings that represent the VM's bytecode.
    Sample input:
    ```.vm=
    4B41545308000000
    010D000000
    010A000000
    15
    0102000000
    18
    1A0100
    1A0000
    04
    ```
3.  **Parser (Frontend)**: Reads the `.vbm` formatted file, validates its header, and translates the bytecode into a high-level, in-memory Intermediate Representation (IR).
4.  **Intermediate Representation (IR)**: A vector of `Instruction` structs (e.g., `ICONST`, `IADD`), which is platform-independent.
5.  **MIPS Generator (Backend)**: Takes the IR and translates each instruction into its corresponding MIPS assembly code sequence.
6.  **Output File (`.s`)**: A text file containing the final MIPS assembly code, ready to be run in a MIPS simulator like MARS or SPIM.
    Sample output for the input in the first point:
    ```assembly=
    .data
    newline: .asciiz "\n"
    .text
    .globl main

    main:
        # ICONST
        li $t0, 13
        sw $t0, 0($sp)
        addiu $sp, $sp, -4
        # ICONST
        li $t0, 10
        sw $t0, 0($sp)
        addiu $sp, $sp, -4
        # AND
        # ICONST
        li $t0, 2
        sw $t0, 0($sp)
        addiu $sp, $sp, -4
        # SHL
        # SYSCALL
        # SYSCALL
        # RET
        li $v0, 10
        syscall

        # Default exit
        li $v0, 10
        syscall

    ```



## Components

The project is broken down into three main components:

### 1. Parser (`parser.cpp`, `parser.hpp`)

The parser acts as the **compiler's frontend**. Its primary responsibility is to read the source `.vbm` code and perform lexical and syntactic analysis to produce the Intermediate Representation. It handles:
- Reading hexadecimal strings from the input code.
- Validating the "KATS" magic number and instruction count in the header.
- Translating each bytecode opcode and its operands into an `Instruction` struct.

### 2. MIPS Generator (`mips_generator.cpp`, `mips_generator.hpp`)

The generator is the **compiler's backend**. It takes the platform-agnostic IR from the parser and generates code for a specific target architecture, which in this case is MIPS. It translates instructions like `ICONST` and `IADD` into low-level MIPS assembly for stack manipulation and arithmetic.

### 3. Main Driver (`main.cpp`)

This file orchestrates the compilation process. It initializes the parser to create the IR from the input file and then passes that IR to the MIPS generator to produce the final assembly output file.

## Input File Format 

The program expects an input text with a specific format:
- The **first line** must be the header, containing a 4-byte magic number ("KATS") and a 4-byte little-endian integer for the instruction count.
- **Subsequent lines** each contain the hexadecimal representation of a single VM instruction.

**Example: `program.txt`**
```
4B41545303000000    # Header: "KATS", 3 instructions
010A000000          # ICONST 10
0119000000          # ICONST 25
02                  # IADD
```

## How to Compile and Run

- Clone the repository using the following command
    ```bash!
    git clone https://github.com/SaiMadhavChakrala/MIPS_Virtual_Machine/
    ```
- Compile the code by running the following commands at the root directory of the repository
    ```bash!
    cd Parser/src
    g++ main.cpp parser.cpp mips_generator.cpp -o vm_parser -std=c++17
    ```
- Write the output from the assembler or custom vm byte code in the program.txt inside the src folder.
- Run the following to generate output.s which contains the MIPS assembly
    ```bash
    ./vm_parser
    ```

## **Contributions**
Sai Madhav(**CS22B060**): Worked on developing the Parser.cpp to transform VM bytecode into the Intermediate Representation (e.g., ICONST, IADD).

Sampath(**CS22B033**): Worked on developing the mips_generator.cpp which transforms the IR into MIPS assembly code.

Adithya Raghuveer(**CS22B019**): Implemented main.cpp to handle input/output . Also extended Parser.cpp to support a wider range of instructions.

## Future work
Currently the VM translated the .vbm/byte code from a .txt file to MIPS assembly instructions. In the upcoming modules it will be extended to convert the generated assembly instructions to MIPS machine code while supporting a wider range of instructions.
