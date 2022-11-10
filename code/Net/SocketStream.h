/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class Socket;

/*! Socket stream wrapper.
 * \ingroup Net
 */
class T_DLLCLASS SocketStream : public IStream
{
	T_RTTI_CLASS;

public:
	SocketStream(Socket* socket, bool readAllowed = true, bool writeAllowed = true, int32_t timeout = -1);

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

	void setAccess(bool readAllowed, bool writeAllowed);

	void setTimeout(int32_t timeout);

private:
	mutable Ref< Socket > m_socket;
	bool m_readAllowed;
	bool m_writeAllowed;
	int32_t m_timeout;
	int64_t m_offset;
};

	}
}

