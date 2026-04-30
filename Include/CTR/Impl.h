/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GUARD_CTR_IMPL_H
#define GUARD_CTR_IMPL_H

#define impl_CTR_LIKELY(x) (bool)__builtin_expect((bool)(x), true)
#define impl_CTR_AS_STRING0(x) #x
#define impl_CTR_AS_STRING(x) impl_CTR_AS_STRING0(x)

#endif /* GUARD_CTR_IMPL_H */