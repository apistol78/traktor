/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/OutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! StringOutputStream buffer.
 * \ingroup Core
 */
class StringOutputStreamBuffer : public OutputStreamBuffer
{
public:
	StringOutputStreamBuffer(size_t initialCapacity = 1024);

	virtual ~StringOutputStreamBuffer();

	bool empty() const;

	std::wstring str() const;

	const wchar_t* c_str() const;

	void reset();

	virtual int32_t overflow(const wchar_t* buffer, int32_t count);

private:
	friend class StringOutputStream;

	wchar_t m_internal[128];
	wchar_t* m_buffer;
	size_t m_initialCapacity;
	size_t m_capacity;
	size_t m_tail;
};

/*! Formatting string stream.
 * \ingroup Core
 */
class T_DLLCLASS StringOutputStream : public OutputStream
{
	T_RTTI_CLASS;

public:
	StringOutputStream(size_t initialCapacity = 1024);

	virtual ~StringOutputStream();

	bool empty() const;

	std::wstring str() const;

	const wchar_t* c_str() const;

	void reset();

private:
	StringOutputStreamBuffer m_buffer;
};

}
