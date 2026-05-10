/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GUARD_CTR_SYNC_H
#define GUARD_CTR_SYNC_H

#include <CTR/Defs.h>

typedef struct CTRMtxImpl CTRMtx;
typedef struct CTRCVImpl CTRCV;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void ctrYield(void);

CTRMtx* ctrMtxCreate(void);
void ctrMtxDestroy(CTRMtx* mtx);
void ctrMtxAcquire(CTRMtx* mtx);
void ctrMtxRelease(CTRMtx* mtx);

CTRCV* ctrCVCreate(void);
void ctrCVDestroy(CTRCV* cv);
void ctrCVWait(CTRCV* cv, CTRMtx* mtx);
void ctrCVNotify(CTRCV* cv, size_t count);
void ctrCVBroadcast(CTRCV* cv);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GUARD_CTR_SYNC_H */