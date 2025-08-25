# Virtual Machine – Status Report


--- 
### Overview
This project implements a **stack-based Virtual Machine (VM)** that translates high-level VM instructions(`.vm file`) into **MIPS assembly** (`.asm`) code.  
The system provides a basic mapping between VM instructions and their MIPS equivalents, enabling execution on MIPS processors or simulators.

---

### Current Capabilities
- Translates **arithmetic commands** (`add`, `eq`,  `gt`, `and`, `or`, e.t.c) to MIPS instructions.
- Handles **memory access** (`push`, `pop`) across basic segments (`constant`, `local`, `this`, `that`, e.t.c).
- Generates **branching commands** (`label`, `goto`, `if-goto`).
- Produces **valid MIPS assembly files** with:
  - **Directives** (`.text`, `.data`, `.global main`)
  - **Labels** for function entry points and branching.
  - **System call usage** for I/O (if implemented later).

---

### File Roles
- **`VMTranslator.py`**  
  Entry point. Takes `.vm` input file(s) and produces `.asm` MIPS output.  
  Orchestrates parsing and code writing.

- **`Parser.py`**  
  - Reads VM source line by line.  
  - Strips whitespace & comments.  
  - Classifies command types (`C_ARITHMETIC`, `C_PUSH`, `C_POP`, etc.).  
  - Extracts arguments.

- **`CodeWriter.py`**  
  - Core translation logic.  
  - Converts VM commands into MIPS assembly.  
  - Manages labels, jumps, function calls, and return sequences.

- **`Main.asm` (output)**  
  - Generated MIPS assembly file.  
  - Includes translated instructions with function labels and logic.

---

### MIPS Instructions Currently Generated
**1. Arithmetic & Logic Commands**
- `add` → `addu $t0, $t1, $t2`
- `sub` → `subu $t0, $t1, $t2`
- `neg` → `subu $t0, $zero, $t1`
- `eq`  → compares two values, pushes result
- `gt`  → greater-than check
- `and` → `and $t0, $t1, $t2`
- `or`  → `or $t0, $t1, $t2`
- `not` → `nor $t0, $t1, $zero`

**2. Memory Access Commands**
- `push constant x` → load constant into stack
- `push local x` / `pop local x`
- `push argument x` / `pop argument x`
- `push this x` / `pop this x`

**3. Program Flow Commands**
- `label X` → translates to `(X)`
- `goto X` → unconditional branch to label
- `if-goto X` → branch if top of stack ≠ 0

---
### Contributions
- CS22B019: `codegen_mips.py`, `lexer.py`
- CS22B033: `SimpleAdd.vm`(example input), `cli.py`
- CS22B060: `parser.py`, `ir.py` , `runtime_mips.s`. 

Currently the VM is converting .vm files to .asm. The appropriate changes will the made in the upcoming modules. 

