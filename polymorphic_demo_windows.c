/*
 * Polymorphic Code Demonstration - Windows Version
 *
 * Educational demonstration of self-modifying code for malware detection research.
 * This program modifies itself on each execution to demonstrate polymorphic behavior.
 *
 * Purpose: Train ML/LLM models to detect polymorphic code patterns
 * Payload: Benign - prints system information and demonstrates code mutation
 *
 * Compilation (MinGW): gcc -o polymorphic_demo.exe polymorphic_demo_windows.c
 * Compilation (MSVC): cl /GS- polymorphic_demo_windows.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEY_SIZE 8
#define CUSTOM_SECTION ".poly"
#define ORIG_KEY "\x01\x01\x01\x01\x01\x01\x01\x01"

// Section attribute for different compilers
#ifdef _MSC_VER
    #pragma section(CUSTOM_SECTION, read, write, execute)
    #define POLYMORPHIC __declspec(allocate(CUSTOM_SECTION))
#else
    #define POLYMORPHIC __attribute__((section(CUSTOM_SECTION)))
#endif

// Global encryption key
static unsigned char key[KEY_SIZE + 1] = ORIG_KEY;

// Function prototypes
void die(char* msg);
IMAGE_SECTION_HEADER* find_section(void* data, const char* name);
void mutate_executable(char* exe_path);
void xor_crypt(unsigned char* data, int len);
void xor_crypt_additional(unsigned char* data, int len, unsigned char* additional_key);
void encrypt_file_sections(void* mapped_data, unsigned char* file_key);
void demonstrate_polymorphism();

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    printf("=== Polymorphic Code Demonstration ===\n");
    printf("Educational research on self-modifying executables\n\n");

    // Get own executable path
    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) {
        die("Could not get executable path");
    }

    printf("Executable: %s\n", exe_path);
    printf("Current key: ");
    for (int i = 0; i < KEY_SIZE; i++) {
        printf("%02x ", (unsigned char)key[i]);
    }
    printf("\n\n");

    // Execute the polymorphic function
    demonstrate_polymorphism();

    // Mutate for next execution
    printf("\n--- Performing Self-Mutation ---\n");
    mutate_executable(exe_path);
    printf("✓ Executable mutated successfully\n");
    printf("✓ New encryption key generated\n");
    printf("✓ Next execution will have different binary signature\n");

    return 0;
}

void die(char* msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    fprintf(stderr, "Last error: %lu\n", GetLastError());
    exit(EXIT_FAILURE);
}

IMAGE_SECTION_HEADER* find_section(void* data, const char* name) {
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)data;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        return NULL;
    }

    IMAGE_NT_HEADERS* nt_headers = (IMAGE_NT_HEADERS*)((BYTE*)data + dos_header->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        return NULL;
    }

    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt_headers);
    int num_sections = nt_headers->FileHeader.NumberOfSections;

    for (int i = 0; i < num_sections; i++) {
        if (strncmp((char*)sections[i].Name, name, IMAGE_SIZEOF_SHORT_NAME) == 0) {
            return &sections[i];
        }
    }

    return NULL;
}

void xor_crypt(unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= (key[i % KEY_SIZE] - 1);
    }
}

void xor_crypt_additional(unsigned char* data, int len, unsigned char* additional_key) {
    for (int i = 0; i < len; i++) {
        data[i] ^= (additional_key[i % KEY_SIZE] - 1);
    }
}

void encrypt_file_sections(void* mapped_data, unsigned char* file_key) {
    // Encrypt additional sections in the file (beyond .poly)
    // This increases the amount of encrypted content for more polymorphism
    const char* sections_to_encrypt[] = {".rdata", NULL};  // Don't encrypt .data (contains our keys!)
    int encrypted_count = 0;
    DWORD total_bytes = 0;

    printf("Encrypting additional file sections:\n");

    for (int i = 0; sections_to_encrypt[i] != NULL; i++) {
        IMAGE_SECTION_HEADER* section = find_section(mapped_data, sections_to_encrypt[i]);
        if (section && section->SizeOfRawData > 0) {
            unsigned char* section_data = (unsigned char*)mapped_data + section->PointerToRawData;

            // Encrypt with file key (XOR is symmetric, so encrypt/decrypt are the same)
            xor_crypt_additional(section_data, section->SizeOfRawData, file_key);

            printf("  ✓ Section %-10s (offset: 0x%06x, size: %6u bytes)\n",
                   sections_to_encrypt[i], section->PointerToRawData, section->SizeOfRawData);
            encrypted_count++;
            total_bytes += section->SizeOfRawData;
        }
    }

    printf("Total sections encrypted: %d (%u bytes)\n", encrypted_count, total_bytes);
}

void mutate_executable(char* exe_path) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = NULL;
    void* mapped_data = NULL;
    DWORD file_size = 0;

    // Open executable file
    hFile = CreateFileA(exe_path, GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        die("Could not open executable file");
    }

    file_size = GetFileSize(hFile, NULL);
    if (file_size == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        die("Could not get file size");
    }

    // Create file mapping
    hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMapping == NULL) {
        CloseHandle(hFile);
        die("Could not create file mapping");
    }

    // Map view of file
    mapped_data = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (mapped_data == NULL) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        die("Could not map view of file");
    }

    // Find .data section (where our key variable is stored)
    IMAGE_SECTION_HEADER* data_section = find_section(mapped_data, ".data");
    if (data_section == NULL) {
        UnmapViewOfFile(mapped_data);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        die("Could not find .data section");
    }

    // Find custom section
    IMAGE_SECTION_HEADER* poly_section = find_section(mapped_data, CUSTOM_SECTION);
    if (poly_section == NULL) {
        UnmapViewOfFile(mapped_data);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        die("Could not find polymorphic section");
    }

    printf("Found section '%s' at offset 0x%x, size 0x%x\n",
           CUSTOM_SECTION, poly_section->PointerToRawData, poly_section->SizeOfRawData);

    // Get pointer to key in file's .data section
    unsigned char* key_in_file = (unsigned char*)mapped_data + data_section->PointerToRawData;

    // Copy current key from file to memory (for decryption)
    memcpy(key, key_in_file, KEY_SIZE);

    printf("Current key from file: ");
    for (int i = 0; i < KEY_SIZE; i++) {
        printf("%02x ", key[i]);
    }
    printf("\n");

    // Get pointer to section data in file
    unsigned char* section_data = (unsigned char*)mapped_data + poly_section->PointerToRawData;

    // Decrypt with old key
    xor_crypt(section_data, poly_section->SizeOfRawData);

    // Generate new random key for .poly section
    printf("Generating new .poly encryption key: ");
    for (int i = 0; i < KEY_SIZE; i++) {
        key[i] = (unsigned char)(rand() % 255);
        printf("%02x ", key[i]);
    }
    printf("\n");

    // Update key in the file's .data section so next execution can decrypt
    memcpy(key_in_file, key, KEY_SIZE);

    // Re-encrypt with new key
    xor_crypt(section_data, poly_section->SizeOfRawData);

    // Generate new file encryption key for additional sections
    unsigned char file_key[KEY_SIZE];
    printf("Generating new file encryption key: ");
    for (int i = 0; i < KEY_SIZE; i++) {
        file_key[i] = (unsigned char)(rand() % 255);
        printf("%02x ", file_key[i]);
    }
    printf("\n\n");

    // Encrypt additional sections in the file before writing
    encrypt_file_sections(mapped_data, file_key);

    printf("\n--- Writing Encrypted File ---\n");

    // Flush changes to disk
    if (!FlushViewOfFile(mapped_data, 0)) {
        fprintf(stderr, "Warning: Could not flush view of file\n");
    }

    // Cleanup
    UnmapViewOfFile(mapped_data);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

// This function is placed in a custom section and gets encrypted
POLYMORPHIC
void demonstrate_polymorphism() {
    printf("--- Polymorphic Function Executing ---\n");

    // Demonstrate that code is running
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    printf("System Information:\n");
    printf("  Processor Architecture: %u\n", si.wProcessorArchitecture);
    printf("  Number of Processors: %lu\n", si.dwNumberOfProcessors);
    printf("  Page Size: %lu bytes\n", si.dwPageSize);

    // Get current process info
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        printf("  Process Memory Usage: %lu KB\n", pmc.WorkingSetSize / 1024);
    }

    printf("\nThis function's code is encrypted on disk.\n");
    printf("Each execution creates a different binary signature.\n");
}
