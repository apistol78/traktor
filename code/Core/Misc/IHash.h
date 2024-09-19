/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Hash function interface.
 * \ingroup Core
 */
class T_DLLCLASS IHash : public Object
{
	T_RTTI_CLASS;

public:
	/*! Begin feeding data for hash calculation. */
	virtual void begin() = 0;

	/*! Feed data to hash calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	virtual void feedBuffer(const void* buffer, uint64_t bufferSize) = 0;

	/*! End feeding data for hash calculation. */
	virtual void end() = 0;

	//@{

	template < typename T >
	void feed(const T& value)
	{
		feedBuffer(&value, sizeof(value));
	}

	void feed(bool value)
	{
		const uint8_t ub = value ? 0xff : 0x00;
		feedBuffer(&ub, sizeof(ub));
	}

	void feed(const wchar_t* value)
	{
		if (value != nullptr)
		{
			for (const wchar_t* ch = value; *ch != 0; ++ch)
				feed(*ch);
		}
	}

	void feed(const std::wstring& value)
	{
		if (!value.empty())
			feedBuffer(value.c_str(), (uint64_t)(value.length() * sizeof(wchar_t)));
	}

	void feed(const std::wstring_view& value)
	{
		for (wchar_t ch : value)
			feed(ch);
	}

	//@}
};

}

