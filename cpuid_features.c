/*
MIT License

Copyright (c) 2026 Marcos Azevedo (psylinux)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
  How to compile: clang -O2 -Wall -Wextra -o cpuid_features cpuid_features.c
*/

static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(subleaf));
}

static void get_vendor_id(char out[13])
{
    // CPUID(0): EBX, EDX, ECX form the 12-byte vendor ID string.
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);

    memcpy(out + 0, &ebx, 4);
    memcpy(out + 4, &edx, 4);
    memcpy(out + 8, &ecx, 4);
    out[12] = '\0';
}

static void get_brand_string(char out[49])
{
    // Brand string via CPUID leaves 0x80000002..0x80000004 (48 bytes total).
    uint32_t max_ext, ebx, ecx, edx;
    cpuid(0x80000000u, 0, &max_ext, &ebx, &ecx, &edx);

    memset(out, 0, 49);

    if (max_ext < 0x80000004u) {
        snprintf(out, 49, "(brand string not supported)");
        return;
    }

    uint32_t eax;
    uint32_t *p = (uint32_t *)out;

    cpuid(0x80000002u, 0, &eax, &ebx, &ecx, &edx);
    *p++ = eax; *p++ = ebx; *p++ = ecx; *p++ = edx;

    cpuid(0x80000003u, 0, &eax, &ebx, &ecx, &edx);
    *p++ = eax; *p++ = ebx; *p++ = ecx; *p++ = edx;

    cpuid(0x80000004u, 0, &eax, &ebx, &ecx, &edx);
    *p++ = eax; *p++ = ebx; *p++ = ecx; *p++ = edx;

    out[48] = '\0';
}

static void print_feature(const char *name, int supported, const char *description)
{
    printf("%-28s: %s  - %s\n",
           name,
           supported ? "SUPPORTED" : "not supported",
           description);
}

int main(void)
{
    uint32_t eax, ebx, ecx, edx;

    // Vendor + max basic leaf
    char vendor[13];
    get_vendor_id(vendor);

    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    uint32_t max_basic_leaf = eax;

    // Brand string + max extended leaf
    cpuid(0x80000000u, 0, &eax, &ebx, &ecx, &edx);
    uint32_t max_extended_leaf = eax;

    char brand[49];
    get_brand_string(brand);

    printf("Vendor ID        : %s\n", vendor);
    printf("Brand String     : %s\n", brand);
    printf("Max Basic Leaf   : 0x%08x\n", max_basic_leaf);
    printf("Max Extended Leaf: 0x%08x\n\n", max_extended_leaf);

    // Leaf 7, subleaf 0: extended features (many modern hardening-related flags)
    if (max_basic_leaf >= 7) {
        cpuid(7, 0, &eax, &ebx, &ecx, &edx);

        // CET (Control-flow Enforcement Technology)
        // CET_SS  = CPUID.(7,0):ECX bit 7
        // CET_IBT = CPUID.(7,0):EDX bit 20
        int cet_ss  = (ecx >> 7)  & 1;   // Shadow Stack
        int cet_ibt = (edx >> 20) & 1;   // Indirect Branch Tracking

        // Other security-relevant features
        int smep = (ebx >> 7)  & 1;      // Supervisor Mode Execution Prevention
        int smap = (ebx >> 20) & 1;      // Supervisor Mode Access Prevention

        printf("CPUID.(7,0): EBX=0x%08x ECX=0x%08x EDX=0x%08x\n", ebx, ecx, edx);
        print_feature("CET_SS (Shadow Stack)", cet_ss, "CET: protects return addresses using a hardware shadow stack");
        print_feature("CET_IBT (Indirect Branch)", cet_ibt, "CET: requires ENDBR64 at valid indirect call/jump targets");
        print_feature("SMEP", smep, "Kernel hardening: prevents executing user pages in supervisor mode");
        print_feature("SMAP", smap, "Kernel hardening: prevents supervisor access to user pages without opt-in");
        printf("\n");
    } else {
        printf("CPUID leaf 7 is not supported on this CPU.\n\n");
    }

    // Leaf 1: classic features (some useful context flags)
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);

    int pae  = (edx >> 6)  & 1;          // Physical Address Extension
    int sse2 = (edx >> 26) & 1;          // Streaming SIMD Extensions 2

    printf("CPUID.(1,0): ECX=0x%08x EDX=0x%08x\n", ecx, edx);
    print_feature("PAE", pae, "Physical Address Extension (historically related to NX in some setups)");
    print_feature("SSE2", sse2, "Vector instruction set (common baseline on x86-64)");
    printf("\n");

    // Leaf 0x80000001: extended features where NX/XD is typically reported
    if (max_extended_leaf >= 0x80000001u) {
        cpuid(0x80000001u, 0, &eax, &ebx, &ecx, &edx);

        int nx = (edx >> 20) & 1;        // NX/XD: No-eXecute / eXecute Disable

        printf("CPUID.(0x80000001,0): EDX=0x%08x\n", edx);
        print_feature("NX/XD", nx, "No-eXecute: allows pages to be marked non-executable (basis for DEP/NX)");
        printf("\n");
    } else {
        printf("CPUID extended leaf 0x80000001 is not supported on this CPU.\n\n");
    }

    return 0;
}
