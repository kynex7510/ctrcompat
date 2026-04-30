/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GUARD_CTR_ASSERT_H
#define GUARD_CTR_ASSERT_H

#include <CTR/Break.h>
#include <CTR/Log.h>
#include <CTR/Impl.h>

/* CTR_ASSERT */

#ifndef NDEBUG

#define CTR_ASSERT(cond)                                                \
    do {                                                                \
        if (!impl_CTR_LIKELY(cond)) {                                   \
            CTR_LOG_DEBUG("Assertion failed: " impl_CTR_AS_STRING(cond) \
                "\nIn file: " __FILE__                                  \
                "\nOn line: " impl_CTR_AS_STRING(__LINE__));            \
            CTR_BREAK();                                                \
        }                                                               \
    } while (false)

#else
#define CTR_ASSERT(cond) (void)((cond))
#endif // !NDEBUG

#endif /* GUARD_CTR_ASSERT_H */