/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <debug.h>
#include <mem_map.h>
#include <arm11/fmt.h>
#include <arm11/allocator/fcram.h>
#include <arm11/allocator/vram.h>
#include <drivers/cache.h>

#include <CTR/Break.h>
#include <CTR/Log.h>
#include <CTR/Allocator.h>

#include <stdarg.h>
#include <malloc.h>
#include <string.h>

void impl_ctr_break() { panic(); }

void impl_ctr_log(const char* fmt, ...) {
    // TODO: ideally we would like to print to the dspico.
    char buf[256];
    va_list args;

    va_start(args, fmt);
    ee_vsnprintf(buf, 256, fmt, args);
    ee_puts(buf);
}

void* ctrAllocAligned(CTRMemType memType, size_t size, size_t alignment) {
    if (!alignment) {
        switch (memType) {
            case CTR_MEM_HEAP:
                return malloc(size);
            case CTR_MEM_LINEAR:
                return fcramAlloc(size);
            case CTR_MEM_VRAM:
                return vramAlloc(size);
            case CTR_MEM_QTMRAM:
                // TODO: return qtmramAlloc(size);
            default:
                return NULL;
        }
    }

    switch (memType) {
        case CTR_MEM_HEAP:
            return memalign(alignment, size);
        case CTR_MEM_LINEAR:
            return fcramMemAlign(size, alignment);
        case CTR_MEM_VRAM:
            return vramMemAlign(size, alignment);
        case CTR_MEM_QTMRAM:
            // TODO: return qtmramMemAlign(size, alignment);
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
            fcramFree(p);
            break;
        case CTR_MEM_VRAM:
            vramFree(p);
            break;
        case CTR_MEM_QTMRAM:
            // TODO: qtmramFree(p);
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

    // TODO: check this.
    if (addr >= AXI_RAM_BASE && addr < A11_HEAP_END)
        return CTR_MEM_HEAP;

    if (addr >= FCRAM_BASE && addr < (FCRAM_BASE + FCRAM_SIZE + FCRAM_EXT_SIZE))
        return CTR_MEM_LINEAR;

    if (addr >= VRAM_BASE && addr < (VRAM_BASE + VRAM_SIZE))
        return CTR_MEM_VRAM;

    if (addr >= QTM_RAM_BASE && addr < (QTM_RAM_BASE + QTM_RAM_SIZE))
        return CTR_MEM_QTMRAM;

    return CTR_MEM_UNKNOWN;
}

CTRVRAMBank ctrGetVRAMBank(const void* p) {
    const u32 addr = (u32)p;

    if (addr >= VRAM_BANK0 && addr < (VRAM_BANK0 + VRAM_BANK_SIZE))
        return CTR_VRAM_BANK_A;

    if (addr >= VRAM_BANK1 && addr < (VRAM_BANK1 + VRAM_BANK_SIZE))
        return CTR_VRAM_BANK_B;

    return CTR_VRAM_BANK_UNKNOWN;
}

size_t ctrGetAllocSize(const void* p) {
    switch (ctrGetMemType(p)) {
        case CTR_MEM_HEAP:
            return malloc_usable_size((void*)p);
        case CTR_MEM_LINEAR:
            return fcramGetSize((void*)p);
        case CTR_MEM_VRAM:
            return vramGetSize((void*)p);
        case CTR_MEM_QTMRAM:
            // TODO: return qtmramGetSize(p);
        default:
            return 0;
    }
}

uintptr_t ctrGetPhysicalAddress(const void* addr) { return (uintptr_t)addr; }
void* ctrGetVirtualAddress(uintptr_t addr) { return (void*)addr; }

// Cache

void ctrInvalidateDataCache(const void* addr, size_t size) { invalidateDCacheRange(addr, size); }
void ctrFlushDataCache(const void* addr, size_t size) { flushDCacheRange(addr, size); }