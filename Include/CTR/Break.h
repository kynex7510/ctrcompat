/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GUARD_CTR_BREAK_H
#define GUARD_CTR_BREAK_H

#include <CTR/Defs.h>
#include <CTR/Log.h>

#define CTR_BREAK() impl_CTR_GLOBAL_NS impl_ctr_break()

#define CTR_BREAK_UNLESS(cond)                                                                  \
    do {                                                                                        \
        if (!impl_CTR_LIKELY(cond)) {                                                           \
            CTR_LOG_DEBUG("Program broke execution: \"" impl_CTR_AS_STRING(cond) "\" was false" \
                "\nIn file: " __FILE__                                                          \
                "\nOn line: " impl_CTR_AS_STRING(__LINE__));                                    \
            CTR_BREAK();                                                                        \
        }                                                                                       \
    } while (false)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

__attribute__((noreturn, cold)) void impl_ctr_break();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GUARD_CTR_BREAK_H */