/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Fixed memory stream.
 * \ingroup Core
 */
class T_DLLCLASS MemoryStream : public IStream
{
	T_RTTI_CLASS;

public:
	explicit MemoryStream(void* buffer, int64_t bufferSize, bool readAllowed = true, bool writeAllowed = true, bool own = false);

	explicit MemoryStream(const void* buffer, int64_t bufferSize);

	virtual ~MemoryStream();

	virtual void close() override;

	virtual bool canRead() const override;

	virtual bool canWrite() const override;

	virtual bool canSeek() const override;

	virtual int64_t tell() const override;

	virtual int64_t available() const override;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override;

	virtual int64_t read(void* block, int64_t nbytes) override;

	virtual int64_t write(const void* block, int64_t nbytes) override;

	virtual void flush() override;

protected:
	uint8_t* m_buffer;
	uint8_t* m_bufferPtr;
	int64_t m_bufferSize;
	bool m_readAllowed;
	bool m_writeAllowed;
	bool m_own;
};

}

