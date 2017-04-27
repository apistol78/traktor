/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_StreamServer_H
#define traktor_net_StreamServer_H

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

private:
	uint16_t m_listenPort;
	Ref< TcpSocket > m_listenSocket;
	Semaphore m_streamsLock;
	std::map< uint32_t, Ref< IStream > > m_streams;
	Thread* m_serverThread;
	std::list< Thread* > m_clientThreads;
	uint32_t m_nextId;

	void threadServer();

	void threadClient(Ref< TcpSocket > clientSocket);
};

	}
}

#endif	// traktor_net_StreamServer_H
