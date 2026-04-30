/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <3ds.h>

#include <CTR/Break.h>
#include <CTR/Log.h>
#include <CTR/Allocator.h>

#include "QTMRAM.h"

#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

// CTR_BREAK

void impl_ctr_break(void) {
    svcBreak(USERBREAK_PANIC);
    while (true) {}
}

// CTR_LOG

void impl_ctr_log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
}

// Allocator

bool qtmramInitRegion(uintptr_t* regionBase, size_t* regionSize) {
    s64 base = 0;
    s64 size = 0;

    if (R_SUCCEEDED(svcGetProcessInfo(&base, CUR_PROCESS_HANDLE, 22))) {
        if (R_SUCCEEDED(svcGetProcessInfo(&size, CUR_PROCESS_HANDLE, 23))) {
            *regionBase = base;
            *regionSize = size;
        }
    }

    return base && size;
}

void* ctrAllocAligned(CTRMemType memType, size_t size, size_t alignment) {
    if (!alignment) {
        switch (memType) {
            case CTR_MEM_HEAP:
                return malloc(size);
            case CTR_MEM_LINEAR:
                return linearAlloc(size);
            case CTR_MEM_VRAM:
                return vramAlloc(size);
            case CTR_MEM_QTMRAM:
                return qtmramAlloc(size);
            default:
                return NULL;
        }
    }

    switch (memType) {
        case CTR_MEM_HEAP:
            return memalign(alignment, size);
        case CTR_MEM_LINEAR:
            return linearMemAlign(size, alignment);
        case CTR_MEM_VRAM:
            return vramMemAlign(size, alignment);
        case CTR_MEM_QTMRAM:
            return qtmramMemAlign(size, alignment);
        default:
            return NULL;
    }
}

static inline vramAllocPos getVRAMPos(CTRVRAMBank bank) {
    if (bank == CTR_VRAM_BANK_A)
        return VRAM_ALLOC_A;

    if (bank == CTR_VRAM_BANK_B)
        return VRAM_ALLOC_B;

    return VRAM_ALLOC_ANY;
}

void* ctrAllocAlignedVRAM(CTRVRAMBank bank, size_t size, size_t aligment) {
    if (!aligment)
        return vramAllocAt(size, getVRAMPos(bank));
    
    return vramMemAlignAt(size, aligment, getVRAMPos(bank));
}

void ctrFree(void* p) {
    switch (ctrGetMemType(p)) {
        case CTR_MEM_HEAP:
            free(p);
            break;
        case CTR_MEM_LINEAR:
            linearFree(p);
            break;
        case CTR_MEM_VRAM:
            vramFree(p);
            break;
        case CTR_MEM_QTMRAM:
            qtmramFree(p);
            break;
        default:;
    }
}

static void* genericRealloc(CTRMemType type, void* p, size_t size) {
    void* q = ctrAlloc(type, size);
    if (q) {
        const size_t oldSize = ctrGetAllocSize(p);
        memcpy(q, p, size < oldSize ? size : oldSize);
        ctrFree(p);
    }

    return q;
}

static inline void* vramReallocCustom(void* p, size_t newSize) {
    const CTRVRAMBank bank = ctrGetVRAMBank(p);

    // If the new size is less than the old size, reallocation must succeed.
    const size_t oldSize = ctrGetAllocSize(p);
    if (newSize < oldSize) {
        ctrFree(p);
        return ctrAllocVRAM(bank, newSize);
    }

    // Try to realloc memory in the same bank first.
    void* q = ctrAllocVRAM(bank, newSize);
    if (!q)
        q = ctrAllocVRAM(bank == CTR_VRAM_BANK_A ? CTR_VRAM_BANK_B : CTR_VRAM_BANK_A, newSize);

    if (q)
        ctrFree(p);

    return q;
}

void* ctrRealloc(void* p, size_t newSize) {
    if (newSize == 0) {
        ctrFree(p);
        return NULL;
    }

    switch (ctrGetMemType(p)) {
        case CTR_MEM_HEAP:
            return realloc(p, newSize);
        case CTR_MEM_LINEAR:
            return genericRealloc(CTR_MEM_LINEAR, p, newSize);
        case CTR_MEM_VRAM:
            return vramReallocCustom(p, newSize);
        case CTR_MEM_QTMRAM:
            return genericRealloc(CTR_MEM_QTMRAM, p, newSize);
        default:
            return NULL;
    }
}

CTRMemType ctrGetMemType(const void* p) {
    const u32 addr = (u32)p;

    if (addr >= OS_HEAP_AREA_BEGIN && addr < OS_HEAP_AREA_END)
        return CTR_MEM_HEAP;

    if (addr >= OS_FCRAM_VADDR && addr < (OS_FCRAM_VADDR + OS_FCRAM_SIZE))
        return CTR_MEM_LINEAR;

    if (addr >= OS_VRAM_VADDR && addr < (OS_VRAM_VADDR + OS_VRAM_SIZE))
        return CTR_MEM_VRAM;

    if (addr >= OS_QTMRAM_VADDR && addr < (OS_QTMRAM_VADDR + OS_QTMRAM_SIZE))
        return CTR_MEM_QTMRAM;

    return CTR_MEM_UNKNOWN;
}

CTRVRAMBank ctrGetVRAMBank(const void* p) {
    const size_t bankSize = OS_VRAM_SIZE / 2;
    const u32 addr = (u32)p;

    if (addr >= OS_VRAM_VADDR && addr < (OS_VRAM_VADDR + bankSize))
        return CTR_VRAM_BANK_A;

    if (addr >= (OS_VRAM_VADDR + bankSize) && addr < (OS_VRAM_VADDR + bankSize * 2))
        return CTR_VRAM_BANK_B;

    return CTR_VRAM_BANK_UNKNOWN;
}

size_t ctrGetAllocSize(const void* p) {
    switch (ctrGetMemType(p)) {
        case CTR_MEM_HEAP:
            return malloc_usable_size((void*)p);
        case CTR_MEM_LINEAR:
            return linearGetSize((void*)p);
        case CTR_MEM_VRAM:
            return vramGetSize((void*)p);
        case CTR_MEM_QTMRAM:
            return qtmramGetSize(p);
        default:
            return 0;
    }
}

uintptr_t ctrGetPhysicalAddress(const void* addr) { return osConvertVirtToPhys(addr); }

void* ctrGetVirtualAddress(uintptr_t addr) {
#define CONVERT_REGION(_name)                                             \
    if (addr >= OS_##_name##_PADDR &&                                     \
        addr < (OS_##_name##_PADDR + OS_##_name##_SIZE))                  \
        return (void*)(addr - (OS_##_name##_PADDR + OS_##_name##_VADDR));

    CONVERT_REGION(FCRAM);
    CONVERT_REGION(VRAM);
    CONVERT_REGION(OLD_FCRAM);
    CONVERT_REGION(DSPRAM);
    CONVERT_REGION(QTMRAM);
    CONVERT_REGION(MMIO);

#undef CONVERT_REGION
    return NULL;
}

// Cache

void ctrInvalidateDataCache(const void* addr, size_t size) {
    // GSP will return an error if the address is not in FCRAM/VRAM.
    const bool fcram = (u32)addr >= OS_FCRAM_VADDR && (u32)addr <= (OS_FCRAM_VADDR + OS_FCRAM_SIZE);
    const bool vram = (u32)addr >= OS_VRAM_VADDR && (u32)addr <= (OS_VRAM_VADDR + OS_VRAM_SIZE);

    if (fcram || vram) {
        CTR_BREAK_UNLESS(R_SUCCEEDED(GSPGPU_InvalidateDataCache(addr, size)));
    }
}

void ctrFlushDataCache(const void* addr, size_t size) {
    // GSP will return an error if the address is not in FCRAM/VRAM.
    const bool fcram = (u32)addr >= OS_FCRAM_VADDR && (u32)addr <= (OS_FCRAM_VADDR + OS_FCRAM_SIZE);
    const bool vram = (u32)addr >= OS_VRAM_VADDR && (u32)addr <= (OS_VRAM_VADDR + OS_VRAM_SIZE);

    if (fcram || vram) {
        CTR_BREAK_UNLESS(R_SUCCEEDED(GSPGPU_FlushDataCache(addr, size)));
    }
}