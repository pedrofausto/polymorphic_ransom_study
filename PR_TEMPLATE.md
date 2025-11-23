# Educational Polymorphic Code Study with Multi-Section Encryption

## Summary

This PR adds comprehensive educational implementations of polymorphic self-modifying executables for malware detection research using ML/LLM models. The code demonstrates advanced polymorphic techniques while maintaining completely benign payloads.

## Key Features

### 🔐 Enhanced Multi-Section Encryption
- **Two-tier encryption system**: Dual random keys per execution
- **Primary encryption**: Custom `.poly` section with polymorphic code
- **File-level encryption**: Additional data sections (`.data`, `.rodata`/`.rdata`)
- **Constructor-based decryption**: Decrypts data sections before `main()` execution
- **Maximum polymorphism**: Encrypts file content in memory before creation

### 🖥️ Cross-Platform Implementations

**Linux (ELF64):**
- `polymorphic_demo_linux.c` - Clean, educational implementation
- Encrypts `.poly`, `.data`, `.rodata`, `.bss` sections
- Uses `mprotect()` for memory permission manipulation
- Constructor function with `__attribute__((constructor))`

**Windows (PE32/PE64):**
- `polymorphic_demo_windows.c` - Clean, educational implementation
- Encrypts `.poly`, `.data`, `.rdata` sections
- Uses `VirtualProtect()` for memory permissions
- Memory-mapped file approach for mutation

### 📚 Comprehensive Documentation
- **README.md**: Quick start guide, examples, research applications
- **TECHNICAL_DOCUMENTATION.md**: In-depth analysis, detection indicators, ML/LLM training guidance
- Clear educational disclaimers and responsible use guidelines

## Technical Implementation

### Polymorphic Behavior
1. Reads own executable into memory
2. Decrypts encrypted sections with current keys
3. Executes benign payload (displays system info)
4. Generates TWO new random encryption keys
5. Re-encrypts `.poly` section with new key #1
6. Encrypts `.data`/`.rodata` sections with new key #2
7. Writes fully encrypted file to disk
8. Next execution: constructor decrypts data sections before main()

**Result**: Different file hash on every execution!

### Detection Indicators for Research

**Behavioral:**
- Self-reference (opens own executable)
- Memory permission changes
- Self-modification (rewrites own file)
- Constructor functions executing before main()
- Multi-key encryption patterns

**Static:**
- Custom section names (`.poly`)
- Dual XOR encryption routines
- Random key generation
- ELF/PE header parsing code

## Research Value

Perfect for training ML/LLM models on:
- ✅ Polymorphic code pattern recognition
- ✅ Behavioral analysis (system calls, API sequences)
- ✅ Static analysis (sections, entropy, headers)
- ✅ Cross-platform technique comparison
- ✅ Constructor-based decryption detection

## Safety & Ethics

- ✅ **Completely benign payloads** - Only displays system information
- ✅ **No malicious functionality** - No file encryption, network activity, privilege escalation
- ✅ **Educational purpose** - Clearly documented for security research
- ✅ **MIT Licensed** - Open source and transparent
- ✅ **Responsible use guidelines** - Clear warnings and ethical guidelines

## Code Quality

- Well-commented and documented
- Cross-compiler support (GCC, MinGW, MSVC)
- Clean, readable implementation
- Educational focus maintained throughout

## Testing

Verified on:
- Linux (ELF64) - Compiles and runs correctly
- File hash changes on each execution
- Multi-section encryption working as expected
- Constructor decryption functioning properly

## Files Changed

- ✨ `polymorphic_demo_linux.c` - Linux implementation with multi-section encryption
- ✨ `polymorphic_demo_windows.c` - Windows implementation with multi-section encryption
- 📝 `README.md` - Comprehensive project documentation
- 📖 `TECHNICAL_DOCUMENTATION.md` - Detailed technical analysis
- 📜 `LICENSE` - MIT License

## Commits

1. **Add educational polymorphic code implementations for both platforms**
   - Initial clean implementations
   - Removed malware aspects while preserving polymorphism
   - Added comprehensive documentation

2. **Implement multi-section file encryption before creation**
   - Enhanced with two-tier encryption system
   - Added file-level section encryption
   - Implemented constructor-based decryption
   - Updated documentation for new features

---

**This PR provides a valuable educational resource for security researchers training detection models on polymorphic malware patterns.**
