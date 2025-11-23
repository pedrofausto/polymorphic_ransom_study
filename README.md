# Polymorphic Code Study

**Educational demonstration of self-modifying executables for malware detection research using Machine Learning and LLMs**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-blue)
![Purpose](https://img.shields.io/badge/purpose-Educational-green)

## ⚠️ Disclaimer

This repository contains educational implementations of polymorphic code techniques. The code is **NOT malicious** - it only modifies itself and contains benign payloads that display system information. This is intended for:

- 🎓 **Security research and education**
- 🤖 **Training ML/LLM models for malware detection**
- 🔍 **Understanding polymorphic behavior patterns**
- 🛡️ **Developing detection mechanisms**

**Do not use this code for malicious purposes.**

---

## Overview

Polymorphic code changes its binary signature on each execution while maintaining functionality. This project demonstrates this technique on both Linux and Windows platforms with completely benign payloads.

### What This Does

1. ✅ Reads its own executable file
2. ✅ Decrypts custom code section (`.poly`)
3. ✅ Executes benign function (displays system info)
4. ✅ Generates TWO new random encryption keys
5. ✅ Re-encrypts `.poly` section with new key
6. ✅ **NEW**: Encrypts additional data sections (`.data`, `.rodata`/`.rdata`) before file creation
7. ✅ Overwrites itself with mutated version
8. ✅ **NEW**: Constructor function decrypts data sections at next startup

**Result**: Different file hash on every execution with multi-section encryption!

---

## Repository Contents

| File | Description |
|------|-------------|
| `polymorphic_demo_linux.c` | Linux/ELF64 implementation (clean, educational) |
| `polymorphic_demo_windows.c` | Windows/PE implementation (clean, educational) |
| `main.c` | Original proof-of-concept (legacy) |
| `TECHNICAL_DOCUMENTATION.md` | Comprehensive technical documentation |
| `LICENSE` | MIT License |

---

## Quick Start

### Linux

```bash
# Compile
gcc -o demo polymorphic_demo_linux.c

# Run and observe mutation
./demo
sha256sum demo

# Run again - different hash!
./demo
sha256sum demo
```

### Windows

```bash
# Compile (MinGW)
gcc -o demo.exe polymorphic_demo_windows.c

# Or with MSVC
cl /GS- polymorphic_demo_windows.c

# Run and observe mutation
demo.exe
certutil -hashfile demo.exe SHA256

# Run again - different hash!
demo.exe
certutil -hashfile demo.exe SHA256
```

---

## How It Works

### Encryption Process

```
[Execution N]                    [Execution N+1]
     |                                 |
     v                                 v
Decrypt with Key_N              Decrypt with Key_N+1
     |                                 |
     v                                 v
Execute Payload                 Execute Payload
     |                                 |
     v                                 v
Generate Key_N+1                Generate Key_N+2
     |                                 |
     v                                 v
Encrypt & Save                  Encrypt & Save
```

### Binary Mutation

Each execution produces a different file:
- **Hash changes** (different file signature)
- **Encrypted bytes change** (new random key)
- **Functionality preserved** (decrypts to same code)

---

## Detection Research

### Behavioral Indicators

These programs exhibit patterns common to polymorphic malware:

```python
patterns = {
    "file_operations": [
        "opens_own_executable",
        "writes_to_self",
        "deletes_and_recreates_self"
    ],
    "memory_operations": [
        "changes_memory_permissions",
        "writes_to_executable_pages"
    ],
    "crypto_operations": [
        "xor_loops",
        "random_key_generation"
    ]
}
```

### Use Cases for ML/LLM Training

1. **Positive Samples**: Polymorphic behavior
2. **Feature Extraction**: System calls, API sequences
3. **Behavioral Analysis**: Runtime patterns
4. **Static Analysis**: Custom sections, entropy
5. **Cross-Platform Comparison**: Linux vs Windows techniques

---

## Platform Differences

| Feature | Linux | Windows |
|---------|-------|---------|
| Format | ELF64 | PE32/PE64 |
| Memory Protection | `mprotect()` | `VirtualProtect()` |
| File Mutation | `unlink()` + recreate | Memory-mapped file |
| Headers | `Elf64_Ehdr` | `IMAGE_NT_HEADERS` |

See [TECHNICAL_DOCUMENTATION.md](TECHNICAL_DOCUMENTATION.md) for detailed comparison.

---

## Documentation

📖 **[Read Full Technical Documentation](TECHNICAL_DOCUMENTATION.md)**

Includes:
- Detailed implementation analysis
- Platform comparisons
- Detection indicators
- ML/LLM training guidance
- Security considerations

---

## Example Output

```
=== Polymorphic Code Demonstration ===
Educational research on self-modifying executables

Executable: ./demo
Size: 17240 bytes
Current key: 7a 3f e2 91 45 bc 28 d3

--- Polymorphic Function Executing ---
System Information:
  System: Linux
  Node: research-vm
  Release: 5.15.0
  Machine: x86_64
  Process ID: 12345

This function's code is encrypted on disk.
Each execution creates a different binary signature.

--- Performing Self-Mutation ---
Found section '.poly' at offset 0x2000, size 0x180
Generating new .poly encryption key: 4b 92 c7 1f 83 5a d4 26
Generating new file encryption key: a3 7e 91 4c b5 22 d8 6f

Encrypting additional file sections:
  ✓ Section .data      (offset: 0x003000, size:    512 bytes)
  ✓ Section .rodata    (offset: 0x003200, size:   1024 bytes)
Total sections encrypted: 2 (1536 bytes)

--- Writing Encrypted File ---
✓ Executable mutated successfully
✓ New encryption keys generated (2 keys)
✓ Next execution will have different binary signature
```

---

## Research Applications

### 1. Machine Learning Training
- **Binary classification**: Polymorphic vs normal
- **Behavioral modeling**: System call sequences
- **Anomaly detection**: Unusual file operations

### 2. LLM-Based Detection
- **Code pattern recognition**: Self-modification techniques
- **Natural language descriptions**: Behavioral summaries
- **Cross-reference analysis**: Multi-platform patterns

### 3. Security Tool Testing
- Test antivirus detection capabilities
- Benchmark sandbox environments
- Evaluate HIPS effectiveness

---

## Safety & Ethics

### This Code is Safe Because:
- ✅ No network functionality
- ✅ No file encryption (except itself)
- ✅ No privilege escalation
- ✅ No persistence mechanisms
- ✅ No data exfiltration
- ✅ Fully documented and transparent

### Responsible Use Guidelines:
- 🔒 Run in isolated VMs or sandboxes
- 📚 Use for education and research only
- 🎓 Attribute in academic papers
- ⚖️ Respect applicable laws and regulations
- ❌ Never use as basis for actual malware

---

## Contributing

Contributions for educational improvements are welcome:
- Enhanced documentation
- Additional platform support
- Detection method examples
- ML model training scripts

Please ensure all contributions maintain the educational focus and benign nature of the project.

---

## License

MIT License - Copyright (c) 2025 Pedro Fausto Rodrigues

See [LICENSE](LICENSE) file for details.

---

## References & Further Reading

- [TECHNICAL_DOCUMENTATION.md](TECHNICAL_DOCUMENTATION.md) - Full technical details
- "Practical Malware Analysis" by Sikorski & Honig
- "The Art of Computer Virus Research and Defense" by Peter Szor
- [ELF Format Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf)
- [PE Format Documentation](https://docs.microsoft.com/en-us/windows/win32/debug/pe-format)

---

## Contact

For questions about this research project, please open an issue on GitHub.

**Remember**: This is educational software. Use responsibly and ethically.
