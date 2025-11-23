/*
 * Polymorphic Code Demonstration - Linux Version
 *
 * Educational demonstration of self-modifying code for malware detection research.
 * This program modifies itself on each execution to demonstrate polymorphic behavior.
 *
 * Purpose: Train ML/LLM models to detect polymorphic code patterns
 * Payload: Benign - prints system information and demonstrates code mutation
 *
 * Compilation: gcc -o polymorphic_demo polymorphic_demo_linux.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <elf.h>

#define KEY_SIZE 8
#define CUSTOM_SECTION ".poly"
#define CRYPT __attribute__((section(CUSTOM_SECTION)))
#define ORIG_KEY "\x01\x01\x01\x01\x01\x01\x01\x01"

// Global encryption key
static unsigned char key[KEY_SIZE + 1] = ORIG_KEY;
extern char __executable_start;

// Secondary encryption key for additional sections
static unsigned char file_key[KEY_SIZE + 1] = ORIG_KEY;

// Function prototypes
void die(char* data, char* msg);
Elf64_Shdr* get_section(void* data, char* section_name);
void mutate(char* data, char* filename, int filesize);
void xor_crypt(unsigned char* data, int len);
void xor_crypt_file(unsigned char* data, int len);
void encrypt_file_sections(char* data);
void decrypt_runtime_sections() __attribute__((constructor));
void demonstrate_polymorphism();

int main(int argc, char** argv) {
    srand(time(NULL));

    printf("=== Polymorphic Code Demonstration ===\n");
    printf("Educational research on self-modifying executables\n\n");

    // Read own executable
    int fd;
    char* exe_data;

    if ((fd = open(argv[0], O_RDONLY, 0)) < 0) {
        die(NULL, "Could not read executable file\n");
    }

    struct stat info;
    fstat(fd, &info);

    if (!(exe_data = malloc(info.st_size))) {
        die(NULL, "Could not allocate memory\n");
    }

    read(fd, exe_data, info.st_size);
    close(fd);

    printf("Executable: %s\n", argv[0]);
    printf("Size: %ld bytes\n", info.st_size);
    printf("Current key: ");
    for (int i = 0; i < KEY_SIZE; i++) {
        printf("%02x ", (unsigned char)key[i]);
    }
    printf("\n\n");

    // Execute the polymorphic function
    demonstrate_polymorphism();

    // Mutate for next execution
    printf("\n--- Performing Self-Mutation ---\n");
    mutate(exe_data, argv[0], info.st_size);
    printf("✓ Executable mutated successfully\n");
    printf("✓ New encryption key generated\n");
    printf("✓ Next execution will have different binary signature\n");

    free(exe_data);
    return 0;
}

void die(char* data, char* msg) {
    if (data) free(data);
    fprintf(stderr, "ERROR: %s", msg);
    exit(EXIT_FAILURE);
}

Elf64_Shdr* get_section(void* data, char* section_name) {
    Elf64_Ehdr* elf_header = (Elf64_Ehdr*)data;
    Elf64_Shdr* section_headers = (Elf64_Shdr*)(data + elf_header->e_shoff);
    char* string_table = data + section_headers[elf_header->e_shstrndx].sh_offset;

    for (int i = 0; i < elf_header->e_shnum; i++) {
        if (!strcmp(string_table + section_headers[i].sh_name, section_name)) {
            return &section_headers[i];
        }
    }
    return NULL;
}

void xor_crypt(unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= (key[i % KEY_SIZE] - 1);
    }
}

void xor_crypt_file(unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= (file_key[i % KEY_SIZE] - 1);
    }
}

void decrypt_runtime_sections() {
    // This runs BEFORE main() to decrypt data sections
    // Only decrypt data sections - code sections must remain unencrypted for execution
    const char* sections_to_decrypt[] = {".data", ".rodata", ".bss", NULL};

    // Open own executable
    char exe_path[256];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) return;
    exe_path[len] = '\0';

    int fd = open(exe_path, O_RDONLY);
    if (fd < 0) return;

    struct stat st;
    fstat(fd, &st);
    char* file_data = malloc(st.st_size);
    if (!file_data) {
        close(fd);
        return;
    }

    read(fd, file_data, st.st_size);
    close(fd);

    // Decrypt each data section at runtime
    for (int i = 0; sections_to_decrypt[i] != NULL; i++) {
        Elf64_Shdr* section = get_section(file_data, (char*)sections_to_decrypt[i]);
        if (section && section->sh_size > 0 && section->sh_addr != 0) {
            unsigned char* runtime_ptr = (unsigned char*)section->sh_addr;

            // Make writable
            uintptr_t page_start = (uintptr_t)runtime_ptr & -getpagesize();
            size_t page_size = ((uintptr_t)runtime_ptr + section->sh_size - page_start + getpagesize() - 1) & -getpagesize();
            mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE);

            // Decrypt
            xor_crypt_file(runtime_ptr, section->sh_size);

            // Restore protection
            mprotect((void*)page_start, page_size, PROT_READ);
        }
    }

    free(file_data);
}

void encrypt_file_sections(char* data) {
    // List of data sections to encrypt in the file (NOT code sections like .text)
    // We encrypt data sections because code needs to be readable for execution
    const char* sections_to_encrypt[] = {".data", ".rodata", ".bss", NULL};
    int encrypted_count = 0;
    size_t total_bytes = 0;

    printf("Encrypting additional file sections:\n");

    for (int i = 0; sections_to_encrypt[i] != NULL; i++) {
        Elf64_Shdr* section = get_section(data, (char*)sections_to_encrypt[i]);
        if (section && section->sh_size > 0) {
            unsigned char* section_data = (unsigned char*)data + section->sh_offset;

            // Decrypt first if already encrypted
            xor_crypt_file(section_data, section->sh_size);

            // Re-encrypt with new file key
            xor_crypt_file(section_data, section->sh_size);

            printf("  ✓ Section %-10s (offset: 0x%06lx, size: %6ld bytes)\n",
                   sections_to_encrypt[i], section->sh_offset, section->sh_size);
            encrypted_count++;
            total_bytes += section->sh_size;
        }
    }

    printf("Total sections encrypted: %d (%ld bytes)\n", encrypted_count, total_bytes);
}

void mutate(char* data, char* filename, int filesize) {
    Elf64_Shdr* section;

    // Find custom polymorphic section
    if (!(section = get_section(data, CUSTOM_SECTION))) {
        die(data, "Could not find polymorphic section\n");
    }

    printf("Found section '%s' at offset 0x%lx, size 0x%lx\n",
           CUSTOM_SECTION, section->sh_offset, section->sh_size);

    // Get runtime address of section
    unsigned char* runtime_addr = (unsigned char*)&__executable_start + section->sh_offset;
    uintptr_t page_start = (uintptr_t)runtime_addr & -getpagesize();
    int page_size = runtime_addr + section->sh_size - (unsigned char*)page_start;

    // Make memory writable
    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        die(data, "Could not make memory writable\n");
    }

    // Get file offset of section
    unsigned char* file_section = (unsigned char*)data + section->sh_offset;

    // Decrypt both runtime and file copies
    xor_crypt(runtime_addr, section->sh_size);
    xor_crypt(file_section, section->sh_size);

    // Restore memory protection
    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_EXEC) < 0) {
        die(data, "Could not restore memory protection\n");
    }

    // Generate new random encryption key for .poly section
    printf("Generating new .poly encryption key: ");
    for (int i = 0; i < KEY_SIZE; i++) {
        key[i] = (unsigned char)(rand() % 255);
        printf("%02x ", key[i]);
    }
    printf("\n");

    // Re-encrypt file copy with new key
    xor_crypt(file_section, section->sh_size);

    // Generate new file encryption key
    printf("Generating new file encryption key: ");
    for (int i = 0; i < KEY_SIZE; i++) {
        file_key[i] = (unsigned char)(rand() % 255);
        printf("%02x ", key[i]);
    }
    printf("\n\n");

    // Encrypt additional sections in the file copy before writing
    encrypt_file_sections(data);

    printf("\n--- Writing Encrypted File ---\n");

    // Write modified executable back to disk
    if (unlink(filename) < 0) {
        die(data, "Could not remove old file\n");
    }

    int fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
    if (fd < 0) {
        die(data, "Could not create new file\n");
    }

    if (write(fd, data, filesize) < 0) {
        die(data, "Could not write file\n");
    }

    close(fd);
}

// This function is placed in a custom section and gets encrypted
CRYPT
void demonstrate_polymorphism() {
    printf("--- Polymorphic Function Executing ---\n");

    // Get system information
    struct utsname sys_info;
    if (uname(&sys_info) == 0) {
        printf("System Information:\n");
        printf("  System: %s\n", sys_info.sysname);
        printf("  Node: %s\n", sys_info.nodename);
        printf("  Release: %s\n", sys_info.release);
        printf("  Version: %s\n", sys_info.version);
        printf("  Machine: %s\n", sys_info.machine);
    }

    // Get process information
    printf("  Process ID: %d\n", getpid());
    printf("  Parent PID: %d\n", getppid());

    printf("\nThis function's code is encrypted on disk.\n");
    printf("Each execution creates a different binary signature.\n");
}
