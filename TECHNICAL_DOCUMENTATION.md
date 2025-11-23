# Polymorphic Code - Technical Documentation

## Overview

This repository contains educational implementations of polymorphic self-modifying code for research purposes. The code demonstrates how executables can mutate themselves to evade signature-based detection, which is valuable for training machine learning models and LLMs to detect such behavior.

**Purpose**: Malware detection research and security education
**Payload**: Benign - displays system information only
**Platforms**: Linux (ELF64) and Windows (PE32/PE64)

---

## What is Polymorphic Code?

Polymorphic code changes its appearance while maintaining its functionality. Each execution produces a different binary signature by:

1. **Encrypting** code sections with random keys
2. **Decrypting** at runtime to execute
3. **Re-encrypting** with a new key before exit
4. **Overwriting** the executable file

This makes the file hash different on each execution, defeating signature-based antivirus detection.

---

## Enhanced Multi-Section Encryption

**NEW**: The implementations now feature advanced polymorphic behavior by encrypting multiple sections of the executable file before writing to disk:

### Two-Tier Encryption System

1. **Primary Encryption (`.poly` section)**
   - Encrypts custom polymorphic code section
   - Uses main encryption key
   - Decrypted at runtime via memory protection manipulation

2. **File-Level Encryption (additional sections)**
   - Encrypts `.data`, `.rodata` (Linux) or `.data`, `.rdata` (Windows)
   - Uses secondary file encryption key
   - Decrypted by constructor function before `main()` executes
   - Maximizes file content encryption while maintaining executability

### How It Works

**Before File Creation:**
```
1. Decrypt .poly section (in memory)
2. Generate NEW random key for .poly
3. Re-encrypt .poly with new key
4. Generate NEW random key for file sections
5. Decrypt .data, .rodata (toggle with old key)
6. Re-encrypt .data, .rodata (toggle with new key)
7. Write entire encrypted file to disk
```

**At Runtime (next execution):**
```
1. Constructor runs BEFORE main()
2. Reads own executable from disk
3. Decrypts .data and .rodata sections in memory
4. main() executes normally with decrypted data
```

### Why This Approach?

- **Code sections (`.text`)** remain unencrypted so OS loader can start the program
- **Data sections** are encrypted on disk, decrypted at runtime
- **Maximum polymorphism** without breaking executability
- **Two independent keys** increase mutation complexity

---

## Implementation Details

### Linux Version (`polymorphic_demo_linux.c`)

#### Compilation
```bash
gcc -o polymorphic_demo polymorphic_demo_linux.c
```

#### How It Works

1. **Self-Reading**
   - Opens itself via `argv[0]`
   - Reads entire executable into memory
   - Parses ELF64 headers

2. **Section Location**
   - Finds custom `.poly` section containing polymorphic function
   - This section is marked with `__attribute__((section(".poly")))`

3. **Runtime Decryption**
   - Uses `mprotect()` to make code pages writable
   - XORs encrypted section with current key
   - Executes decrypted code
   - Restores read-execute permissions

4. **Mutation**
   - Generates new 8-byte random encryption key
   - Re-encrypts section in file copy
   - Overwrites executable (unlink + rewrite)

#### Key System Calls
```c
open(argv[0], O_RDONLY)     // Read own executable
mprotect(..., PROT_RWX)     // Change memory permissions
unlink(filename)            // Delete old file
write(fd, data, size)       // Write mutated version
```

---

### Windows Version (`polymorphic_demo_windows.c`)

#### Compilation

**MinGW/GCC:**
```bash
gcc -o polymorphic_demo.exe polymorphic_demo_windows.c
```

**Microsoft Visual C++:**
```cmd
cl /GS- polymorphic_demo_windows.c
```

#### How It Works

1. **Self-Reading**
   - Gets own path via `GetModuleFileNameA()`
   - Opens file with `CreateFileA()`
   - Creates memory-mapped view

2. **Section Location**
   - Parses PE (Portable Executable) headers
   - Finds custom `.poly` section
   - Uses `IMAGE_DOS_HEADER`, `IMAGE_NT_HEADERS`, `IMAGE_SECTION_HEADER`

3. **File Mapping**
   - Uses `CreateFileMappingA()` + `MapViewOfFile()`
   - Modifies mapped memory directly
   - Changes automatically written to disk via `FlushViewOfFile()`

4. **Mutation**
   - Decrypts section with old key
   - Generates new random key
   - Re-encrypts with new key
   - Flushes changes to file

#### Key Win32 APIs
```c
GetModuleFileNameA()        // Get executable path
CreateFileA()               // Open file
CreateFileMappingA()        // Create file mapping object
MapViewOfFile()             // Map to memory
FlushViewOfFile()           // Write changes to disk
```

---

## Platform Comparison

| Aspect | Linux (ELF) | Windows (PE) |
|--------|-------------|--------------|
| **Executable Format** | ELF64 | PE32/PE64 |
| **Headers** | `Elf64_Ehdr`, `Elf64_Shdr` | `IMAGE_DOS_HEADER`, `IMAGE_NT_HEADERS` |
| **Memory Protection** | `mprotect()` | `VirtualProtect()` |
| **File Operations** | POSIX (`open`, `read`, `write`) | Win32 API (`CreateFile`, etc.) |
| **Mutation Strategy** | Unlink + recreate | Memory-mapped file |
| **Section Attribute** | `__attribute__((section()))` | `__declspec(allocate())` or `#pragma section` |

---

## Encryption Mechanism

### XOR Cipher
```c
void xor_crypt(unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= (key[i % KEY_SIZE] - 1);
    }
}
```

- **Algorithm**: Simple XOR with 8-byte repeating key
- **Key Generation**: `rand() % 255` (pseudorandom)
- **Key Size**: 8 bytes
- **Initial Key**: `\x01\x01\x01\x01\x01\x01\x01\x01`

**Note**: This is not cryptographically secure - it's intentionally simple for educational purposes.

---

## Detection Indicators

### Behavioral Indicators

Both implementations exhibit these suspicious behaviors:

1. **Self-Reference**
   - Opening own executable path for reading
   - Linux: `open(argv[0])`
   - Windows: `GetModuleFileName()` + `CreateFile(own_path)`

2. **Memory Permission Changes**
   - Making code pages writable
   - Linux: `mprotect(..., PROT_WRITE | PROT_EXEC)`
   - Windows: `VirtualProtect(..., PAGE_EXECUTE_READWRITE)`

3. **Self-Modification**
   - Writing to own executable file
   - Deleting and recreating own file

4. **File Integrity Violations**
   - File hash changes after execution
   - Digital signatures become invalid (if signed)

### Static Indicators

1. **Custom Sections**
   - Non-standard section name: `.poly`
   - Section with executable + writable flags

2. **Suspicious Code Patterns**
   - XOR loops over code sections
   - Random key generation
   - ELF/PE header parsing

3. **API Call Sequences**
   - Linux: `open(argv[0])` → `mprotect()` → `unlink()`
   - Windows: `GetModuleFileName()` → `CreateFileMapping()` → `FlushViewOfFile()`

4. **Constructor Functions (NEW)**
   - Functions marked with `__attribute__((constructor))`
   - Execute before `main()` - suspicious for decryption routines
   - Read own executable at startup
   - Modify memory permissions before program logic

5. **Multi-Key Encryption (NEW)**
   - Multiple encryption keys in data sections
   - Dual XOR encryption routines (`xor_crypt` + `xor_crypt_file`)
   - Sequential encryption of multiple sections

---

## Machine Learning Detection

### Feature Extraction

For training ML/LLM models, extract these features:

**Static Features:**
- PE/ELF header anomalies
- Section names and characteristics
- Import/Export table analysis
- String patterns (e.g., encryption keys)
- Code entropy (high entropy = likely encrypted)

**Dynamic Features:**
- System call sequences
- File operations on self
- Memory protection changes
- Network activity (none in these demos)
- File system modifications

**Behavioral Patterns:**
```
open(self) → mprotect(RWX) → write(self) → unlink(self)
```

### Sample Training Labels

```json
{
  "file": "polymorphic_demo",
  "polymorphic": true,
  "self_modifying": true,
  "malicious": false,
  "techniques": [
    "runtime_decryption",
    "code_mutation",
    "xor_encryption",
    "custom_sections"
  ]
}
```

---

## Usage for Research

### Running the Demos

**Linux:**
```bash
# Compile
gcc -o demo polymorphic_demo_linux.c

# Run first time
./demo

# Check file hash
sha256sum demo

# Run again
./demo

# Check hash again - it will be different!
sha256sum demo
```

**Windows:**
```cmd
REM Compile
gcc -o demo.exe polymorphic_demo_windows.c

REM Run first time
demo.exe

REM Check hash
certutil -hashfile demo.exe SHA256

REM Run again
demo.exe

REM Hash will be different!
certutil -hashfile demo.exe SHA256
```

### Observing Mutations

```bash
# Monitor file changes
watch -n 1 'sha256sum polymorphic_demo'

# In another terminal
while true; do ./polymorphic_demo; sleep 2; done
```

### Analyzing with Tools

**Static Analysis:**
```bash
# Linux
readelf -S polymorphic_demo | grep poly
objdump -s -j .poly polymorphic_demo

# Windows
dumpbin /HEADERS demo.exe
dumpbin /SECTION:.poly demo.exe
```

**Dynamic Analysis:**
```bash
# Linux - system call tracing
strace -e open,mprotect,unlink,write ./polymorphic_demo

# Windows - API monitoring
# Use Process Monitor or API Monitor
```

---

## Educational Value

### What This Demonstrates

✅ **Self-modifying code techniques**
✅ **Polymorphic behavior patterns**
✅ **Executable file format manipulation**
✅ **Memory protection mechanisms**
✅ **Cross-platform implementation differences**

### What This is NOT

❌ **Actual malware** - Payload is benign
❌ **Ransomware** - No encryption of user files
❌ **Evasion tool** - Designed for research, not circumvention
❌ **Production code** - Intentionally simplified for education

---

## Research Applications

1. **Training Detection Models**
   - Use as positive samples for polymorphic code
   - Compare with benign executables
   - Train on behavioral patterns

2. **Testing Security Tools**
   - Evaluate antivirus detection capabilities
   - Test HIPS (Host Intrusion Prevention Systems)
   - Benchmark sandboxes and analyzers

3. **Academic Research**
   - Study polymorphic engine design
   - Analyze mutation strategies
   - Develop detection algorithms

4. **Security Training**
   - Teach malware analysis techniques
   - Demonstrate reverse engineering
   - Explain defensive programming

---

## Security Considerations

### Why This is Safe

- **No network activity** - Cannot spread
- **No file encryption** - Only modifies itself
- **No privilege escalation** - Runs with user permissions
- **No persistence** - Doesn't install itself
- **Open source** - Fully transparent behavior

### Responsible Use

- ✅ Use in isolated VMs or sandboxes
- ✅ Share for educational purposes
- ✅ Attribute properly if used in research
- ❌ Don't use as basis for actual malware
- ❌ Don't deploy on production systems without authorization

---

## References

### File Formats
- [ELF Format Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf)
- [PE Format (Microsoft)](https://docs.microsoft.com/en-us/windows/win32/debug/pe-format)

### Polymorphic Techniques
- "The Art of Computer Virus Research and Defense" by Peter Szor
- "Practical Malware Analysis" by Michael Sikorski & Andrew Honig

### Detection Methods
- YARA rules for behavioral patterns
- Cuckoo Sandbox for dynamic analysis
- IDA Pro / Ghidra for static analysis

---

## License

MIT License - See LICENSE file

Copyright (c) 2025 Pedro Fausto Rodrigues

**Disclaimer**: This software is provided for educational and research purposes only.
