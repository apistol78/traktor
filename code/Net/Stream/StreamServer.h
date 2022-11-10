/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <map>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;
class Thread;

	namespace net
	{

class TcpSocket;

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS StreamServer : public Object
{
	T_RTTI_CLASS;

public:
	StreamServer();

	bool create();

	void destroy();

	uint32_t publish(IStream* stream);

	uint16_t getListenPort() const;

	TcpSocket* getListenSocket() const;

	uint32_t getStreamCount() const;

private:
	struct Client
	{
		Ref< TcpSocket > socket;
		Ref< IStream > stream;
		uint32_t streamId;
	};

	uint16_t m_listenPort;
	Ref< TcpSocket > m_listenSocket;
	AlignedVector< Client > m_clients;
	mutable Semaphore m_streamsLock;
	std::map< uint32_t, Ref< IStream > > m_streams;
	Thread* m_serverThread;
	uint32_t m_nextId;

	void threadServer();
};

	}
}

