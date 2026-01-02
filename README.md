# cpuid_features

A small **x86 / x86_64 CPU feature inspection tool** written in C.  
It queries the processor directly using the **CPUID (CPU Identification)** instruction and reports 
support for modern **security and hardening features**, including **CET**, **IBT**, and **Shadow Stack**.

This project is intended for **low-level learning**, **reverse engineering**, **exploit research**, and **systems programming**.

---

## What this tool does

The program queries several CPUID leaves and prints:

### CPU identification
- Vendor ID (e.g. `GenuineIntel`)
- Full brand string (model name)
- Maximum supported CPUID leaves

### Security-related CPU features
- **CET_SS (Shadow Stack)**  
  Hardware-enforced protection for return addresses.
- **CET_IBT (Indirect Branch Tracking)**  
  Requires `ENDBR64` at valid indirect call/jump targets.
- **SMEP (Supervisor Mode Execution Prevention)**  
  Prevents kernel execution of user-space memory.
- **SMAP (Supervisor Mode Access Prevention)**  
  Prevents kernel access to user-space memory without opt-in.
- **NX / XD (No-eXecute / eXecute Disable)**  
  Allows marking memory pages as non-executable.

These features are commonly encountered when analyzing:
- modern Linux binaries
- hardened toolchains
- exploit mitigations
- `endbr64` instructions
- CET-related metadata in ELF files

---

## Files

```text
.
├── cpuid_features.c   # Main source code
├── Makefile           # Build and utility targets
└── README.md          # This file
````

---

## Requirements

* **Architecture:** x86 or x86_64
* **Compiler:**
  * macOS: `clang` (default)
  * Linux: `gcc` or `clang`
* **OS:** macOS or Linux

> On non-x86 architectures (e.g. ARM / Apple Silicon), this program will **not work**.

---

## Build and Run

```bash
make
make run
```

## Example output

```text
Vendor ID        : GenuineIntel
Brand String     : Intel(R) Core(TM) i9-9980HK CPU @ 2.40GHz
Max Basic Leaf   : 0x00000016
Max Extended Leaf: 0x80000008

CPUID.(7,0): EBX=0x029c6fbf ECX=0x40000000 EDX=0xbc000e00
CET_SS (Shadow Stack)      : not supported
CET_IBT (Indirect Branch)  : not supported
SMEP                       : SUPPORTED
SMAP                       : SUPPORTED

CPUID.(1,0): ECX=0x7ffafbff EDX=0xbfebfbff
PAE                        : SUPPORTED
SSE2                       : SUPPORTED

CPUID.(0x80000001,0): EDX=0x2c100800
NX/XD                     : SUPPORTED
```

---

## Notes on CET and `endbr64`

* **CET (Control-flow Enforcement Technology)** is only enforced if:

  1. The CPU supports it
  2. The OS enables it
  3. The binary is built to use it

* If your CPU does **not** support CET:

  * `endbr64` behaves like a no-op
  * Patching it out will not change runtime behavior

This tool helps you determine **which layer is missing**.

---

## Intended audience

This project is useful for people working with:

* exploit development
* reverse engineering
* binary hardening analysis
* kernel / firmware research
* low-level C and assembly
* understanding modern mitigation techniques

---

## License

MIT License.
See the LICENSE file for details.
