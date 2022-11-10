/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
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

/*! Dynamic memory stream wrapper.
 * \ingroup Core
 */
class T_DLLCLASS DynamicMemoryStream : public IStream
{
	T_RTTI_CLASS;

public:
	explicit DynamicMemoryStream(AlignedVector< uint8_t >& buffer, bool readAllowed = true, bool writeAllowed = true);

	explicit DynamicMemoryStream(bool readAllowed = true, bool writeAllowed = true);

	virtual void close() override final;

	virtual bool canRead() const override final;

	virtual bool canWrite() const override final;

	virtual bool canSeek() const override final;

	virtual int64_t tell() const override final;

	virtual int64_t available() const override final;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override final;

	virtual int64_t read(void* block, int64_t nbytes) override final;

	virtual int64_t write(const void* block, int64_t nbytes) override final;

	virtual void flush() override final;

	const AlignedVector< uint8_t >& getBuffer() const;

	AlignedVector< uint8_t >& getBuffer();

private:
	AlignedVector< uint8_t > m_internal;
	AlignedVector< uint8_t >* m_buffer;
	int64_t m_readPosition;
	bool m_readAllowed;
	bool m_writeAllowed;
};

}

