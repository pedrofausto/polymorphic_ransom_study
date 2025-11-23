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

#### Python Version (Recommended)

```bash
# Install dependencies
pip install anthropic openai  # Optional, for LLM API support

# Make executable
chmod +x llm_backup_classifier.py
```

#### C Version

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
python3 llm_backup_classifier.py

# Scan specific directory
python3 llm_backup_classifier.py /path/to/scan
```

#### With Anthropic Claude

```bash
# Set API key
export ANTHROPIC_API_KEY="your-api-key-here"

# Run with Anthropic
python3 llm_backup_classifier.py --llm-provider anthropic
```

#### With OpenAI GPT

```bash
# Set API key
export OPENAI_API_KEY="your-api-key-here"

# Run with OpenAI
python3 llm_backup_classifier.py --llm-provider openai
```

#### Classification Only (No Backup)

```bash
# Analyze without creating backups
python3 llm_backup_classifier.py --no-backup
```

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

#### Anthropic Claude

```python
# In llm_backup_classifier.py
import anthropic

client = anthropic.Anthropic(api_key=os.getenv('ANTHROPIC_API_KEY'))
message = client.messages.create(
    model="claude-3-5-sonnet-20241022",
    max_tokens=2000,
    messages=[{"role": "user", "content": prompt}]
)
```

#### OpenAI GPT

```python
import openai

openai.api_key = os.getenv('OPENAI_API_KEY')
response = openai.ChatCompletion.create(
    model="gpt-4",
    messages=[
        {"role": "system", "content": "You are a file classification assistant."},
        {"role": "user", "content": prompt}
    ]
)
```

#### Local LLM (Ollama)

```python
# Example integration with Ollama
import requests

response = requests.post('http://localhost:11434/api/generate', json={
    'model': 'llama2',
    'prompt': prompt
})
```

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

```python
# Add to _classify_rule_based()
custom_keywords = ['project_alpha', 'internal_only']

for keyword in custom_keywords:
    if keyword in filename_lower:
        file.is_sensitive = True
        file.reason = f"Custom rule: {keyword}"
```

#### Integration with Backup Systems

```python
# Example: Copy to cloud storage
import boto3

def backup_to_s3(file_path, bucket_name):
    s3 = boto3.client('s3')
    s3.upload_file(file_path, bucket_name, file_path)
```

#### Custom LLM Providers

```python
def _classify_custom_llm(self) -> int:
    """Integrate your own LLM service"""
    # Your implementation here
    pass
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
