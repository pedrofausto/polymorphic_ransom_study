#!/usr/bin/env python3
"""
LLM-Based File Classifier for Automated Backups

Educational demonstration of using AI to identify potentially sensitive files
for automated backup creation based on filename analysis.

Purpose: Research on AI-driven data classification and backup automation
Action: Creates backup copies (NO encryption, NO file modification)

Usage:
    python3 llm_backup_classifier.py [directory] [--llm-provider openai|anthropic|local]
"""

import os
import sys
import json
import shutil
import argparse
from datetime import datetime
from pathlib import Path
from typing import List, Dict, Optional

# Optional: Import LLM libraries (install as needed)
try:
    import anthropic
    HAS_ANTHROPIC = True
except ImportError:
    HAS_ANTHROPIC = False

try:
    import openai
    HAS_OPENAI = True
except ImportError:
    HAS_OPENAI = False

BACKUP_DIR = "./sensitive_backups"


class FileInfo:
    """Represents a file and its classification status"""
    def __init__(self, filename: str, filepath: str):
        self.filename = filename
        self.filepath = filepath
        self.is_sensitive = False
        self.reason = ""
        self.confidence = 0.0


class LLMBackupClassifier:
    """Main classifier that uses LLM to identify sensitive files"""

    def __init__(self, llm_provider: str = "rule-based"):
        self.llm_provider = llm_provider
        self.files: List[FileInfo] = []

    def scan_directory(self, directory: str) -> int:
        """Scan directory and collect file information"""
        print(f"Scanning directory: {directory}")

        path = Path(directory)
        if not path.exists() or not path.is_dir():
            print(f"Error: {directory} is not a valid directory")
            return 0

        for item in path.iterdir():
            if item.is_file():
                # Skip hidden files and the script itself
                if not item.name.startswith('.') and item.suffix != '.py':
                    file_info = FileInfo(item.name, str(item))
                    self.files.append(file_info)

        print(f"Found {len(self.files)} files to analyze\n")
        return len(self.files)

    def classify_with_llm(self) -> int:
        """Classify files using LLM or rule-based approach"""
        print("--- Analyzing files with LLM ---")

        if self.llm_provider == "anthropic" and HAS_ANTHROPIC:
            return self._classify_anthropic()
        elif self.llm_provider == "openai" and HAS_OPENAI:
            return self._classify_openai()
        elif self.llm_provider == "local":
            return self._classify_local_llm()
        else:
            return self._classify_rule_based()

    def _classify_rule_based(self) -> int:
        """Rule-based classification (demonstration/fallback)"""
        print("⚠️  Using rule-based classification (LLM API not configured)")
        print("   For production, configure API keys for actual LLM integration\n")

        sensitive_keywords = [
            'tax', 'bank', 'passport', 'ssn', 'confidential',
            'private', 'secret', 'password', 'key', 'invoice',
            'contract', 'nda', 'medical', 'legal', 'financial',
            'salary', 'personal', 'encrypted', 'backup'
        ]

        sensitive_extensions = [
            '.key', '.pem', '.p12', '.pfx', '.gpg', '.asc'
        ]

        sensitive_count = 0

        for file in self.files:
            filename_lower = file.filename.lower()

            # Check keywords
            for keyword in sensitive_keywords:
                if keyword in filename_lower:
                    file.is_sensitive = True
                    file.reason = f"Contains keyword: {keyword}"
                    file.confidence = 0.8
                    sensitive_count += 1
                    break

            # Check extensions
            if not file.is_sensitive:
                for ext in sensitive_extensions:
                    if filename_lower.endswith(ext):
                        file.is_sensitive = True
                        file.reason = f"Sensitive file type: {ext}"
                        file.confidence = 0.9
                        sensitive_count += 1
                        break

        return sensitive_count

    def _classify_anthropic(self) -> int:
        """Classify using Anthropic Claude API"""
        print("Using Anthropic Claude API for classification\n")

        # Get API key from environment
        api_key = os.getenv('ANTHROPIC_API_KEY')
        if not api_key:
            print("Error: ANTHROPIC_API_KEY not set. Falling back to rule-based.")
            return self._classify_rule_based()

        client = anthropic.Anthropic(api_key=api_key)

        # Build prompt
        prompt = self._build_llm_prompt()

        try:
            message = client.messages.create(
                model="claude-3-5-sonnet-20241022",
                max_tokens=2000,
                messages=[{
                    "role": "user",
                    "content": prompt
                }]
            )

            # Parse response
            response_text = message.content[0].text
            return self._parse_llm_response(response_text)

        except Exception as e:
            print(f"Error calling Anthropic API: {e}")
            print("Falling back to rule-based classification\n")
            return self._classify_rule_based()

    def _classify_openai(self) -> int:
        """Classify using OpenAI GPT API"""
        print("Using OpenAI GPT API for classification\n")

        api_key = os.getenv('OPENAI_API_KEY')
        if not api_key:
            print("Error: OPENAI_API_KEY not set. Falling back to rule-based.")
            return self._classify_rule_based()

        openai.api_key = api_key
        prompt = self._build_llm_prompt()

        try:
            response = openai.ChatCompletion.create(
                model="gpt-4",
                messages=[
                    {"role": "system", "content": "You are a file classification assistant."},
                    {"role": "user", "content": prompt}
                ],
                temperature=0.3
            )

            response_text = response.choices[0].message.content
            return self._parse_llm_response(response_text)

        except Exception as e:
            print(f"Error calling OpenAI API: {e}")
            print("Falling back to rule-based classification\n")
            return self._classify_rule_based()

    def _classify_local_llm(self) -> int:
        """Classify using local LLM (ollama, llama.cpp, etc.)"""
        print("Using local LLM for classification")
        print("⚠️  Local LLM integration not implemented in this demo")
        print("   Implement using ollama, llama.cpp, or other local LLM framework\n")
        return self._classify_rule_based()

    def _build_llm_prompt(self) -> str:
        """Build prompt for LLM classification"""
        filenames = "\n".join([f"{i+1}. {f.filename}" for i, f in enumerate(self.files)])

        prompt = f"""You are a file classification assistant. Analyze the following filenames and identify which ones MAY contain sensitive or important information that should be backed up.

Consider files that might contain:
- Financial documents (tax returns, bank statements, invoices, receipts)
- Personal identification (passport copies, ID scans, certificates)
- Confidential work files (contracts, NDAs, proprietary data)
- Private data (medical records, legal documents)
- Important projects or source code
- Cryptographic keys or credentials

Filenames to analyze:
{filenames}

Respond in JSON format with an array of objects. Each object should have:
- "index": the file number (1-based)
- "sensitive": true or false
- "reason": brief explanation if sensitive (max 50 chars)
- "confidence": 0.0 to 1.0

Example response:
[
  {{"index": 1, "sensitive": true, "reason": "Tax document", "confidence": 0.9}},
  {{"index": 2, "sensitive": false, "reason": "Generic document", "confidence": 0.3}}
]

Return only the JSON array, no additional text."""

        return prompt

    def _parse_llm_response(self, response_text: str) -> int:
        """Parse LLM JSON response and update file classifications"""
        try:
            # Extract JSON from response (handle markdown code blocks)
            json_start = response_text.find('[')
            json_end = response_text.rfind(']') + 1

            if json_start == -1 or json_end == 0:
                raise ValueError("No JSON array found in response")

            json_str = response_text[json_start:json_end]
            classifications = json.loads(json_str)

            sensitive_count = 0
            for item in classifications:
                index = item.get('index', 0) - 1  # Convert to 0-based
                if 0 <= index < len(self.files):
                    self.files[index].is_sensitive = item.get('sensitive', False)
                    self.files[index].reason = item.get('reason', '')
                    self.files[index].confidence = item.get('confidence', 0.5)

                    if self.files[index].is_sensitive:
                        sensitive_count += 1

            return sensitive_count

        except Exception as e:
            print(f"Error parsing LLM response: {e}")
            print("Response:", response_text[:200])
            return 0

    def create_backups(self) -> int:
        """Create backup copies of sensitive files"""
        # Create backup directory
        backup_path = Path(BACKUP_DIR)
        backup_path.mkdir(exist_ok=True)

        print(f"\n--- Creating Backups ---")
        print(f"Backup directory: {BACKUP_DIR}\n")

        backed_up = 0
        for file in self.files:
            if file.is_sensitive:
                # Create timestamped backup filename
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                backup_filename = f"{timestamp}_{file.filename}"
                backup_filepath = backup_path / backup_filename

                try:
                    shutil.copy2(file.filepath, backup_filepath)
                    print(f"📄 {file.filename}")
                    print(f"   Reason: {file.reason} (confidence: {file.confidence:.1%})")
                    print(f"   ✅ Backed up to: {backup_filepath}")
                    backed_up += 1
                except Exception as e:
                    print(f"   ❌ Error: {e}")

        return backed_up

    def generate_report(self):
        """Generate classification report"""
        report_path = Path(BACKUP_DIR) / f"classification_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"

        report = {
            "timestamp": datetime.now().isoformat(),
            "llm_provider": self.llm_provider,
            "total_files": len(self.files),
            "sensitive_files": sum(1 for f in self.files if f.is_sensitive),
            "classifications": [
                {
                    "filename": f.filename,
                    "sensitive": f.is_sensitive,
                    "reason": f.reason,
                    "confidence": f.confidence
                }
                for f in self.files
            ]
        }

        with open(report_path, 'w') as f:
            json.dump(report, f, indent=2)

        print(f"\n📊 Classification report saved: {report_path}")


def main():
    parser = argparse.ArgumentParser(description='LLM-Based File Classifier for Automated Backups')
    parser.add_argument('directory', nargs='?', default='.', help='Directory to scan (default: current directory)')
    parser.add_argument('--llm-provider', choices=['rule-based', 'anthropic', 'openai', 'local'],
                        default='rule-based', help='LLM provider to use (default: rule-based)')
    parser.add_argument('--no-backup', action='store_true', help='Classify only, do not create backups')

    args = parser.parse_args()

    print("=== LLM-Based File Classification for Backup ===")
    print("Educational demonstration of AI-driven data identification")
    print("Action: Identifies sensitive files and creates backup copies")
    print("Note: Original files remain unchanged\n")

    classifier = LLMBackupClassifier(llm_provider=args.llm_provider)

    # Scan directory
    file_count = classifier.scan_directory(args.directory)
    if file_count == 0:
        print("No files found. Exiting.")
        return

    # Classify files
    sensitive_count = classifier.classify_with_llm()

    if sensitive_count == 0:
        print("\nNo sensitive files identified. No backups needed.")
        return

    print(f"\nIdentified {sensitive_count} potentially sensitive files")

    # Create backups (unless disabled)
    if not args.no_backup:
        backed_up = classifier.create_backups()
        print(f"\n✅ Backup completed: {backed_up} files backed up to {BACKUP_DIR}/")
        print("   Original files remain unchanged.")

    # Generate report
    classifier.generate_report()


if __name__ == "__main__":
    main()
