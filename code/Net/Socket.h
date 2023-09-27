/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

enum IoctlCommand
{
	IccNonBlockingIo,	// FIONBIO
	IccReadPending		// FIONREAD
};

/*! Socket base class.
 * \ingroup Net
 */
class T_DLLCLASS Socket : public Object
{
	T_RTTI_CLASS;

public:
	typedef intptr_t handle_t;

	Socket();

	explicit Socket(handle_t socket_);

	Socket(const Socket&) = delete;

	Socket& operator = (const Socket&) = delete;

	virtual ~Socket();

	/*! Gracefully close socket. */
	virtual void close();

	/*! Query state of socket.
	 *
	 * \param read Check for pending reads.
	 * \param write Check pending writes.
	 * \param except Check for exceptions.
	 * \param timeout Timeout in milliseconds.
	 * \return Non zero if read, write or exception are pending.
	 */
	int select(bool read, bool write, bool except, int timeout);

	/*! Send block of data through socket.
	 *
	 * \param data Pointer to block of data.
	 * \param length Number of bytes.
	 * \return Number of bytes sent, 0 if connection gracefully terminated or -1 connection aborted.
	 */
	int send(const void* data, int length);

	/*! Receive block of data.
	 *
	 * \param data Pointer to block of data.
	 * \param length Number of bytes.
	 * \return Number of bytes received, 0 if connection terminated gracefully or -1 if connection aborted.
	 */
	int recv(void* data, int length);

	/*! Send single byte.
	 *
	 * \param byte Byte to send.
	 * \return Number of bytes sent, 0 if connection gracefully terminated or -1 connection aborted.
	 */
	int send(int byte);

	/*! Read single byte.
	 *
	 * \return Byte value or -1 if connection aborted.
	 */
	int recv();

	/*! Control IO of socket.
	 *
	 * \param cmd IO command.
	 * \param argp IO command argument(s).
	 * \return Result of IO command.
	 */
	bool ioctl(IoctlCommand cmd, unsigned long* argp);

	/*! Get socket handle.
	 *
	 * \return Socket handle.
	 */
	handle_t handle() const;

protected:
	handle_t m_socket;
};

}
