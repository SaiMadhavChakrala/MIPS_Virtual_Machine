# --- CONFIGURATION ---
CROSS_PREFIX = mips-linux-gnu-

# --- TOOLS ---
CXX = $(CROSS_PREFIX)g++
OBJCOPY = $(CROSS_PREFIX)objcopy
OBJDUMP = $(CROSS_PREFIX)objdump

# --- FLAGS ---
CXXFLAGS = -O2 -march=r3000 -mfp32 -mno-abicalls -g -Wall -Wextra -I. 

# --- SOURCE FILES ---
CPP_SOURCES = main.cpp parser.cpp mips_generator.cpp \
              mips_assembler.cpp symbol_table.cpp register_allocator.cpp

# --- BUILD DIRECTORIES ---
OBJ_DIR = build/obj
BUILD_DIR = build

# --- OBJECT FILES ---
OBJS = $(CPP_SOURCES:%.cpp=$(OBJ_DIR)/%.o)

# --- TARGETS ---
KERNEL_ELF  = $(BUILD_DIR)/program_r3000.elf
KERNEL_BIN  = $(BUILD_DIR)/program_r3000.bin
KERNEL_HEX  = $(BUILD_DIR)/program_r3000.hex
DISASSEMBLY = $(BUILD_DIR)/disassembly_r3000.txt

# --- DEFAULT ---
all: $(KERNEL_BIN) $(KERNEL_HEX) $(DISASSEMBLY)

# --- Compile C++ files ---
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Link ELF ---
$(KERNEL_ELF): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lstdc++ -lc
	@echo "ELF created: $@"

# --- Create binary ---
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# --- Create hex file ---
$(KERNEL_HEX): $(KERNEL_BIN)
	od -t x4 -An -w4 -v $< | tr -d ' ' > $@

# --- Disassembly ---
$(DISASSEMBLY): $(KERNEL_ELF)
	@echo "Generating disassembly..."
	$(OBJDUMP) -d $< > $@

# --- Clean ---
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
