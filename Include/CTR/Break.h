/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GUARD_CTR_BREAK_H
#define GUARD_CTR_BREAK_H

#define CTR_BREAK() impl_ctr_break()

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

__attribute__((noreturn, cold)) void impl_ctr_break();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GUARD_CTR_BREAK_H */