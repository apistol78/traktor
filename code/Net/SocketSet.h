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
#include "Core/Ref.h"
#include "Core/RefArray.h"

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

/*! Set of sockets.
 * \ingroup Net
 *
 * Enable queries on a set of socket
 * without explicitly checking each socket.
 */
class T_DLLCLASS SocketSet : public Object
{
	T_RTTI_CLASS;

public:
	void add(Socket* socket);

	void remove(Socket* socket);

	int32_t count() const;

	Ref< Socket > get(int32_t index) const;

	bool contain(Socket* socket) const;

	int select(bool read, bool write, bool except, int timeout, SocketSet& outResultSet);

private:
	RefArray< Socket > m_sockets;
};

	}
}

