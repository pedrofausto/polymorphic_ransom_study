/*
 * LLM-Based File Classifier for Automated Backups
 *
 * Educational demonstration of using AI to identify potentially sensitive files
 * for automated backup creation based on filename analysis.
 *
 * Purpose: Research on AI-driven data classification and backup automation
 * Action: Creates backup copies (NO encryption, NO file modification)
 *
 * Compilation: gcc -o llm_backup llm_backup_classifier.c -lcurl -ljson-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <time.h>

#define MAX_FILES 1000
#define MAX_FILENAME_LEN 256
#define BACKUP_DIR "./sensitive_backups"

// Structure to hold file information
typedef struct {
    char filename[MAX_FILENAME_LEN];
    char filepath[MAX_FILENAME_LEN * 2];
    int is_sensitive;
    char reason[512];
} FileInfo;

// For storing CURL response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Function prototypes
void scan_directory(const char* dir_path, FileInfo* files, int* file_count);
int classify_files_with_llm(FileInfo* files, int file_count);
void create_backup(FileInfo* file);
void create_backup_directory();
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
char* build_llm_prompt(FileInfo* files, int file_count);

int main(int argc, char** argv) {
    FileInfo files[MAX_FILES];
    int file_count = 0;

    printf("=== LLM-Based File Classification for Backup ===\n");
    printf("Educational demonstration of AI-driven data identification\n\n");

    // Get directory to scan (default to current directory)
    const char* scan_dir = (argc > 1) ? argv[1] : ".";

    printf("Scanning directory: %s\n", scan_dir);

    // Scan directory for files
    scan_directory(scan_dir, files, &file_count);
    printf("Found %d files to analyze\n\n", file_count);

    if (file_count == 0) {
        printf("No files found. Exiting.\n");
        return 0;
    }

    // Classify files using LLM
    printf("--- Analyzing files with LLM ---\n");
    int sensitive_count = classify_files_with_llm(files, file_count);

    if (sensitive_count == 0) {
        printf("\nNo sensitive files identified. No backups needed.\n");
        return 0;
    }

    printf("\n--- Creating Backups ---\n");
    printf("Identified %d potentially sensitive files\n\n", sensitive_count);

    // Create backup directory
    create_backup_directory();

    // Create backups of sensitive files
    int backed_up = 0;
    for (int i = 0; i < file_count; i++) {
        if (files[i].is_sensitive) {
            printf("📄 %s\n", files[i].filename);
            printf("   Reason: %s\n", files[i].reason);
            create_backup(&files[i]);
            backed_up++;
        }
    }

    printf("\n✅ Backup completed: %d files backed up to %s/\n", backed_up, BACKUP_DIR);
    printf("   Original files remain unchanged.\n");

    return 0;
}

void scan_directory(const char* dir_path, FileInfo* files, int* file_count) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;

    dir = opendir(dir_path);
    if (!dir) {
        perror("Could not open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL && *file_count < MAX_FILES) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build full path
        char full_path[MAX_FILENAME_LEN * 2];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // Get file stats
        if (stat(full_path, &file_stat) == -1) {
            continue;
        }

        // Only process regular files (not directories)
        if (S_ISREG(file_stat.st_mode)) {
            strncpy(files[*file_count].filename, entry->d_name, MAX_FILENAME_LEN - 1);
            strncpy(files[*file_count].filepath, full_path, sizeof(files[*file_count].filepath) - 1);
            files[*file_count].is_sensitive = 0;
            files[*file_count].reason[0] = '\0';
            (*file_count)++;
        }
    }

    closedir(dir);
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("Not enough memory\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char* build_llm_prompt(FileInfo* files, int file_count) {
    // Build a prompt for the LLM with the list of filenames
    size_t prompt_size = 4096 + (file_count * MAX_FILENAME_LEN);
    char* prompt = malloc(prompt_size);

    snprintf(prompt, prompt_size,
        "You are a file classification assistant. Analyze the following filenames and identify which ones "
        "MAY contain sensitive or important information that should be backed up. Consider:\n"
        "- Financial documents (tax, bank, invoices, receipts)\n"
        "- Personal documents (passport, ID, certificates)\n"
        "- Confidential work files (contracts, NDAs, proprietary data)\n"
        "- Private data (medical records, legal documents)\n"
        "- Important projects or source code\n\n"
        "Filenames to analyze:\n");

    for (int i = 0; i < file_count; i++) {
        char file_line[MAX_FILENAME_LEN + 10];
        snprintf(file_line, sizeof(file_line), "%d. %s\n", i + 1, files[i].filename);
        strncat(prompt, file_line, prompt_size - strlen(prompt) - 1);
    }

    strncat(prompt,
        "\nRespond in JSON format with an array of objects. Each object should have:\n"
        "- \"index\": the file number (1-based)\n"
        "- \"sensitive\": true or false\n"
        "- \"reason\": brief explanation (if sensitive)\n\n"
        "Example: [{\"index\": 1, \"sensitive\": true, \"reason\": \"Tax document\"}]\n",
        prompt_size - strlen(prompt) - 1);

    return prompt;
}

int classify_files_with_llm(FileInfo* files, int file_count) {
    // NOTE: This is a placeholder demonstration
    // In a real implementation, you would:
    // 1. Call an actual LLM API (OpenAI, Anthropic, local LLM, etc.)
    // 2. Parse the JSON response
    // 3. Update the FileInfo structures

    printf("⚠️  LLM API integration placeholder\n");
    printf("   In production, this would call an LLM API endpoint\n");
    printf("   For demonstration, using rule-based classification\n\n");

    // Demonstration: Simple rule-based classification
    // In production, replace with actual LLM API call
    int sensitive_count = 0;

    const char* sensitive_keywords[] = {
        "tax", "bank", "passport", "ssn", "confidential",
        "private", "secret", "password", "key", "invoice",
        "contract", "nda", "medical", "legal", "financial",
        NULL
    };

    for (int i = 0; i < file_count; i++) {
        char lowercase_name[MAX_FILENAME_LEN];
        strncpy(lowercase_name, files[i].filename, MAX_FILENAME_LEN - 1);

        // Convert to lowercase for comparison
        for (int j = 0; lowercase_name[j]; j++) {
            lowercase_name[j] = tolower(lowercase_name[j]);
        }

        // Check for sensitive keywords
        for (int k = 0; sensitive_keywords[k] != NULL; k++) {
            if (strstr(lowercase_name, sensitive_keywords[k]) != NULL) {
                files[i].is_sensitive = 1;
                snprintf(files[i].reason, sizeof(files[i].reason),
                        "Contains keyword: %s", sensitive_keywords[k]);
                sensitive_count++;
                break;
            }
        }
    }

    return sensitive_count;
}

void create_backup_directory() {
    struct stat st = {0};

    if (stat(BACKUP_DIR, &st) == -1) {
        mkdir(BACKUP_DIR, 0700);
        printf("Created backup directory: %s\n\n", BACKUP_DIR);
    }
}

void create_backup(FileInfo* file) {
    char backup_path[MAX_FILENAME_LEN * 3];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // Create backup filename with timestamp
    snprintf(backup_path, sizeof(backup_path),
            "%s/%04d%02d%02d_%02d%02d%02d_%s",
            BACKUP_DIR,
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            file->filename);

    // Copy file to backup location
    FILE *src = fopen(file->filepath, "rb");
    if (!src) {
        printf("   ❌ Error: Could not open source file\n");
        return;
    }

    FILE *dst = fopen(backup_path, "wb");
    if (!dst) {
        printf("   ❌ Error: Could not create backup file\n");
        fclose(src);
        return;
    }

    // Copy contents
    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);

    printf("   ✅ Backed up to: %s\n", backup_path);
}

/*
 * PRODUCTION LLM INTEGRATION EXAMPLE (commented out):
 *
 * int classify_files_with_llm_real(FileInfo* files, int file_count) {
 *     CURL *curl;
 *     CURLcode res;
 *     struct MemoryStruct chunk;
 *
 *     chunk.memory = malloc(1);
 *     chunk.size = 0;
 *
 *     curl_global_init(CURL_GLOBAL_ALL);
 *     curl = curl_easy_init();
 *
 *     if(curl) {
 *         char* prompt = build_llm_prompt(files, file_count);
 *
 *         // Build JSON request
 *         struct json_object *jobj = json_object_new_object();
 *         json_object_object_add(jobj, "prompt", json_object_new_string(prompt));
 *         json_object_object_add(jobj, "max_tokens", json_object_new_int(2000));
 *
 *         const char* json_str = json_object_to_json_string(jobj);
 *
 *         // Set CURL options
 *         curl_easy_setopt(curl, CURLOPT_URL, "https://api.anthropic.com/v1/complete");
 *         curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
 *         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
 *         curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
 *
 *         // Perform request
 *         res = curl_easy_perform(curl);
 *
 *         if(res == CURLE_OK) {
 *             // Parse response and update files
 *             // ... implementation ...
 *         }
 *
 *         curl_easy_cleanup(curl);
 *         free(prompt);
 *         json_object_put(jobj);
 *     }
 *
 *     free(chunk.memory);
 *     curl_global_cleanup();
 *
 *     return sensitive_count;
 * }
 */
