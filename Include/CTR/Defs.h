/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GUARD_CTR_DEFS_H
#define GUARD_CTR_DEFS_H

#include <stdbool.h>

#ifdef __cplusplus
#define CTR_INLINE inline
#else
#define CTR_INLINE static inline
#endif // __cplusplus

/* Private stuff */

#define impl_CTR_LIKELY(x) (bool)__builtin_expect((bool)(x), true)
#define impl_CTR_AS_STRING0(x) #x
#define impl_CTR_AS_STRING(x) impl_CTR_AS_STRING0(x)

#ifdef __cplusplus
#define impl_CTR_GLOBAL_NS ::
#else
#define impl_CTR_GLOBAL_NS
#endif

#endif /* GUARD_CTR_DEFS_H */