/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Primitive writer class.
 * \ingroup Core
 */
class T_DLLCLASS Writer : public Object
{
	T_RTTI_CLASS;

public:
	Writer(IStream* stream);

	Writer& operator << (bool v);

	Writer& operator << (int8_t c);

	Writer& operator << (uint8_t uc);

	Writer& operator << (int16_t i);

	Writer& operator << (uint16_t ui);

	Writer& operator << (int32_t i);

	Writer& operator << (uint32_t i);

	Writer& operator << (int64_t i);

	Writer& operator << (uint64_t i);

	Writer& operator << (float f);

	Writer& operator << (double f);

	Writer& operator << (const std::wstring& s);

	Writer& operator << (const wchar_t* s);

	int64_t write(const void* block, int64_t nbytes);

	int64_t write(const void* block, int64_t count, int64_t size);

private:
	Ref< IStream > m_stream;
};

}

