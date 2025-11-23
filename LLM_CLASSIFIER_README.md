## LLM-Based File Classifier for Automated Backups

### Overview

This tool demonstrates how Large Language Models (LLMs) can be used to identify potentially sensitive files for automated backup purposes. It's an educational project showing AI-driven data classification without any encryption or file modification.

### ⚠️ Important Disclaimers

- **Read-only Analysis**: Only examines filenames, never modifies original files
- **Backup Only**: Creates copies in a separate directory
- **No Encryption**: Backups are unencrypted copies
- **Educational Purpose**: Designed for research on AI-based data classification
- **No Malware**: This is NOT ransomware - it only creates backup copies

### Features

✅ **LLM-Based Classification**
- Uses AI to analyze filenames for sensitivity indicators
- Supports multiple LLM providers (Anthropic, OpenAI, local)
- Fallback to rule-based classification

✅ **Automated Backup Creation**
- Creates timestamped backup copies
- Preserves original files unchanged
- Organized backup directory structure

✅ **Classification Reporting**
- Generates JSON reports with confidence scores
- Tracks classification decisions
- Useful for ML/LLM research

### Installation

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libcurl4-openssl-dev libjson-c-dev

# Compile
gcc -o llm_backup llm_backup_classifier.c -lcurl -ljson-c
```

### Usage

#### Basic Usage (Rule-Based)

```bash
# Scan current directory
./llm_backup

# Scan specific directory
./llm_backup /path/to/scan
```

#### Extending with LLM API

The C implementation includes commented template code for integrating LLM APIs. To enable:

1. **Uncomment the LLM integration section** in the source code
2. **Implement API calls** using libcurl (examples provided)
3. **Parse JSON responses** using json-c library
4. **Recompile** with your changes

See the commented `classify_files_with_llm_real()` function in the source code for a template.

### How It Works

#### 1. **File Discovery**
```
Scans directory → Collects filenames → Filters regular files
```

#### 2. **LLM Classification**
```
Build prompt with filenames
        ↓
Send to LLM API (or use rules)
        ↓
Parse JSON response
        ↓
Mark sensitive files
```

#### 3. **Backup Creation**
```
For each sensitive file:
  - Generate timestamp
  - Copy to backup directory
  - Preserve metadata
  - Log action
```

#### 4. **Report Generation**
```
Create JSON report with:
  - All classifications
  - Confidence scores
  - Reasoning
```

### Example Output

```
=== LLM-Based File Classification for Backup ===
Educational demonstration of AI-driven data identification
Action: Identifies sensitive files and creates backup copies
Note: Original files remain unchanged

Scanning directory: /home/user/documents
Found 25 files to analyze

--- Analyzing files with LLM ---
Using Anthropic Claude API for classification

Identified 5 potentially sensitive files

--- Creating Backups ---
Backup directory: ./sensitive_backups

📄 tax_return_2024.pdf
   Reason: Tax document (confidence: 95.0%)
   ✅ Backed up to: ./sensitive_backups/20250123_143022_tax_return_2024.pdf

📄 passport_scan.jpg
   Reason: Personal identification (confidence: 98.0%)
   ✅ Backed up to: ./sensitive_backups/20250123_143023_passport_scan.jpg

📄 contract_confidential.docx
   Reason: Confidential work file (confidence: 92.0%)
   ✅ Backed up to: ./sensitive_backups/20250123_143024_contract_confidential.docx

✅ Backup completed: 5 files backed up to ./sensitive_backups/
   Original files remain unchanged.

📊 Classification report saved: ./sensitive_backups/classification_report_20250123_143025.json
```

### Classification Criteria

The LLM analyzes filenames for indicators of:

**Financial Data:**
- Tax returns, W-2 forms
- Bank statements, invoices
- Financial reports, receipts

**Personal Information:**
- ID scans, passport copies
- Birth certificates, diplomas
- Medical records

**Confidential Work Files:**
- Contracts, NDAs
- Proprietary documents
- Source code for sensitive projects

**Cryptographic Material:**
- Private keys (.key, .pem)
- Certificates (.p12, .pfx)
- Encrypted archives

### Research Applications

#### 1. **AI-Driven Data Classification**
- Study LLM accuracy in identifying sensitive data
- Compare different models' classification performance
- Benchmark against rule-based systems

#### 2. **Privacy Protection Research**
- Understand what filenames leak information
- Develop better naming conventions
- Test data loss prevention systems

#### 3. **Automated Backup Systems**
- Research intelligent backup strategies
- Optimize storage by backing up only important files
- Study user data organization patterns

#### 4. **Security Awareness**
- Demonstrate what information filenames reveal
- Educational tool for data security training
- Show importance of file naming practices

### API Configuration

#### LLM Integration Template (in C)

The source code includes a commented template for API integration:

```c
/*
 * PRODUCTION LLM INTEGRATION EXAMPLE (commented out in source):
 */
int classify_files_with_llm_real(FileInfo* files, int file_count) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        char* prompt = build_llm_prompt(files, file_count);

        // Build JSON request
        struct json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "prompt", json_object_new_string(prompt));

        // Set API endpoint and headers
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.anthropic.com/v1/complete");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);

        // Perform request and parse response
        res = curl_easy_perform(curl);

        // ... handle response ...
    }

    return sensitive_count;
}
```

Uncomment and customize this function for production LLM integration.

### File Structure

```
./sensitive_backups/
├── 20250123_143022_tax_return_2024.pdf
├── 20250123_143023_passport_scan.jpg
├── 20250123_143024_contract_confidential.docx
└── classification_report_20250123_143025.json
```

### Classification Report Format

```json
{
  "timestamp": "2025-01-23T14:30:25.123456",
  "llm_provider": "anthropic",
  "total_files": 25,
  "sensitive_files": 5,
  "classifications": [
    {
      "filename": "tax_return_2024.pdf",
      "sensitive": true,
      "reason": "Tax document",
      "confidence": 0.95
    },
    {
      "filename": "random_notes.txt",
      "sensitive": false,
      "reason": "Generic document",
      "confidence": 0.2
    }
  ]
}
```

### Safety Features

1. **No File Modification**
   - Original files are never altered
   - Only reads filenames (not content)
   - Creates copies, doesn't move files

2. **Explicit Backup Directory**
   - All backups go to `./sensitive_backups/`
   - Clear separation from originals
   - Easy to review before committing

3. **Timestamped Copies**
   - Each backup has timestamp
   - No overwriting previous backups
   - Full audit trail

4. **Dry-Run Mode**
   - Use `--no-backup` to classify without copying
   - Review decisions before creating backups

### Limitations

- **Filename-Only Analysis**: Doesn't examine file contents
- **False Positives**: May flag non-sensitive files with sensitive-sounding names
- **False Negatives**: May miss sensitive files with generic names
- **Context-Dependent**: What's "sensitive" varies by user/organization

### Extending the Tool

#### Custom Classification Rules

Add custom keywords to the `classify_files_with_llm()` function:

```c
// In classify_files_with_llm()
const char* custom_keywords[] = {
    "project_alpha", "internal_only", "confidential",
    NULL
};

// Add to existing keyword check loop
for (int k = 0; custom_keywords[k] != NULL; k++) {
    if (strstr(lowercase_name, custom_keywords[k]) != NULL) {
        files[i].is_sensitive = 1;
        snprintf(files[i].reason, sizeof(files[i].reason),
                "Custom rule: %s", custom_keywords[k]);
        sensitive_count++;
        break;
    }
}
```

#### Integration with Backup Systems

Modify the `create_backup()` function to integrate with external systems:

```c
void create_backup(FileInfo* file) {
    // ... existing backup code ...

    // Add cloud storage integration
    // system("aws s3 cp backup_path s3://bucket/");
    // Or call cloud SDK APIs
}
```

#### Custom LLM Integration

Implement your own LLM service in the commented template function:

```c
int classify_with_custom_llm(FileInfo* files, int file_count) {
    // Your custom LLM service integration
    // Use libcurl for HTTP requests
    // Parse responses with json-c
    return sensitive_count;
}
```

### License

MIT License - See LICENSE file

### Responsible Use

✅ **Appropriate Uses:**
- Research on AI classification systems
- Educational demonstrations
- Personal backup automation
- Security awareness training

❌ **Inappropriate Uses:**
- Unauthorized access to others' files
- Building malware or ransomware
- Violating privacy regulations
- Data exfiltration

### Contributing

Contributions welcome for:
- Additional LLM provider integrations
- Improved classification algorithms
- Better filename analysis
- Enhanced reporting features

---

**Remember**: This tool is for educational and research purposes. Always respect privacy and data protection regulations.
