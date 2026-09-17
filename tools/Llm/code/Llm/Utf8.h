/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::llm
{

/*! Length of the prefix of \a text that no further bytes can change.
 * \ingroup Llm
 *
 * A model emits text one token at a time, and a token can carry part of a
 * character; this reports how much of what has arrived is safe to show. Only
 * a truncated but still valid sequence at the very end is held back. Bytes
 * that can never begin or continue a character are not held back, because
 * nothing arriving later will repair them.
 */
size_t T_DLLCLASS getValidUtf8Length(const std::string& text);

/*! Widen UTF-8 \a text, substituting U+FFFD for anything malformed.
 * \ingroup Llm
 *
 * Model output is not guaranteed to be well formed: a byte fallback token
 * carries a single byte, which on its own is often not a character. This
 * always terminates and always produces a valid result, which the encoding
 * aware conversions in Core do not for truncated input.
 */
std::wstring T_DLLCLASS widenUtf8(const std::string& text);

}
