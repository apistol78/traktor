/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/IEncoding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! UTF-8 encoding.
 * \ingroup Core
 */
class T_DLLCLASS Utf8Encoding : public IEncoding
{
	T_RTTI_CLASS;

public:
	Utf8Encoding() = default;

	virtual int translate(const wchar_t* chars, int32_t count, uint8_t* out) const override final;

	virtual int translate(const uint8_t in[MaxEncodingSize], int32_t count, wchar_t& out) const override final;
};

}

