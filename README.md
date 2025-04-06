<!-- display-subdirectories: true -->

# ThreatPlayground

This repository contains a collection of malware code samples, lab configurations, and analysis techniques for **educational, research, and reverse engineering purposes only**.

## NOTE TO ALL

This repository will undergo major revamp due to changes made in another project. Please do not fork it under it is completed.

## ⚠️ IMPORTANT DISCLAIMER

**WARNING: These files implement potentially harmful techniques and should ONLY be used in safe, isolated environments such as a virtual machine with no network access. The author assumes NO responsibility for any damage caused by the misuse of these files.**

## Purpose

This repository serves strictly educational purposes:

* Demonstrate various malware techniques for understanding threat actors' methods
* Support learning malware analysis and reverse engineering skills
* Share lab setup and configurations for analysis or offensive testing
* Enable cybersecurity research and improve threat detection capabilities

## Contents

The repository includes examples of various techniques, configurations, etc:

* **Keyloggers**: Code demonstrating keystroke capture methods
* **Reverse Shells**: Examples of remote access implementations
* **File Encryptors**: Demonstrations of encryption techniques (similar to those used in ransomware)
* **Process Injectors**: Code illustrating process memory manipulation
* **Windows Server Setup**: Demonstrations of how to setup Windows Server in a lab environment
* **Linux Server Setup**: Demonstrations of how to setup Linux Server in lab environment
* **Other Samples**: Additional security-relevant code examples
* **Supporting Scripts**: Analysis tools and helper utilities

## Repository Structure

```
ThreatPlayground/
├── Malware/
│   └── SimpleKeylogger/
│       ├── info.md
│       ├── code/
│       │   ├── keylogger.h
│       │   └── keylogger.c
│       └── analysis/
│           ├── reverse-engineering.md
│           └── malware-analysis.md
├── Infrastructure/
│       ├── info.md
│       ├── steps/
│       │   └── steps.md
│       └── configuration-files/
│           └── configuration-files.md
└── README.md
```

## Safe Usage Guidelines

1. **Isolation**: ALWAYS use these files in a completely isolated environment:
   - Use a dedicated virtual machine with no valuable data
   - Disable network connectivity when testing
   - Consider using a dedicated physical machine disconnected from networks

2. **Compilation**: Build C/C++ files with appropriate compilers:
   ```bash
   gcc keylogger.c -o keylogger
   gcc reverse_shell.c -o reverse_shell
   ```

3. **Analysis Approach**:
   - Examine code to understand implementation techniques
   - Use debugging tools (GDB, WinDbg) to observe runtime behavior
   - Employ disassemblers and decompilers (IDA Pro, Ghidra, Radare2) for deeper analysis

4. **Python Scripts**: Run the Python utilities with Python 3:
   ```bash
   python3 analyzer.py
   ```

## Safety Precautions

* Create VM snapshots before running any code
* Keep host systems fully patched with security updates
* Maintain updated antivirus/security tools on host systems
* Use dedicated hardware when possible

## Ethical Guidelines

* This repository is strictly for **defensive security research**
* Do not deploy these techniques against systems without explicit permission
* Adhere to all applicable laws and regulations regarding computer security

## Contributing

Contributions such as additional examples, analysis methodologies, or documentation improvements are welcome, provided they:

* Serve educational purposes only
* Include clear documentation and warnings
* Follow ethical security research guidelines
* Do not contain actual malware signatures or functional exploits for unpatched vulnerabilities

## License

This repository is provided as-is without warranty under the MIT License. See LICENSE file for details.

## Acknowledgments

This project draws inspiration from security researchers and educators dedicated to improving defensive security practices through education and transparency.
