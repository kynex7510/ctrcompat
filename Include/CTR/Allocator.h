/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GUARD_CTR_ALLOCATOR_H
#define GUARD_CTR_ALLOCATOR_H

#include <CTR/Defs.h>

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CTR_MEM_HEAP,
    CTR_MEM_LINEAR,
    CTR_MEM_VRAM,
    CTR_MEM_QTMRAM,
    CTR_MEM_UNKNOWN,
} CTRMemType;

typedef enum {
    CTR_VRAM_BANK_A,
    CTR_VRAM_BANK_B,
    CTR_VRAM_BANK_ANY,
    CTR_VRAM_BANK_UNKNOWN,
} CTRVRAMBank;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void* ctrAllocAligned(CTRMemType memType, size_t size, size_t alignment);
void* ctrAllocAlignedVRAM(CTRVRAMBank bank, size_t size, size_t aligment);

CTR_INLINE void* ctrAlloc(CTRMemType memType, size_t size) { return ctrAllocAligned(memType, size, 0); }
CTR_INLINE void* ctrAllocVRAM(CTRVRAMBank bank, size_t size) { return ctrAllocAlignedVRAM(bank, size, 0); }

void ctrFree(void* p);

// newSize == 0 frees the buffer. VRAM reallocation doesn't retain content.
void* ctrRealloc(void* p, size_t newSize);

CTRMemType ctrGetMemType(const void* p);
CTRVRAMBank ctrGetVRAMBank(const void* p);
size_t ctrGetAllocSize(const void* p);

CTR_INLINE bool ctrIsHeap(const void* p) { return ctrGetMemType(p) == CTR_MEM_HEAP; }
CTR_INLINE bool ctrIsLinear(const void* p) { return ctrGetMemType(p) == CTR_MEM_LINEAR; }
CTR_INLINE bool ctrIsVRAM(const void* p) { return ctrGetMemType(p) == CTR_MEM_VRAM; }
CTR_INLINE bool ctrIsQTMRAM(const void* p) { return ctrGetMemType(p) == CTR_MEM_QTMRAM; }

uintptr_t ctrGetPhysicalAddress(const void* addr);
void* ctrGetVirtualAddress(uintptr_t addr);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GUARD_CTR_ALLOCATOR_H */