/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_Connection_H
#define traktor_db_Connection_H

#include <map>
#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{

class Semaphore;
class Thread;

	namespace net
	{

class BidirectionalObjectTransport;
class StreamServer;
class TcpSocket;

	}

	namespace db
	{

class IMessage;
class IMessageListener;
class IProviderDatabase;

/*! \brief Client connection.
 * \ingroup Database
 */
class Connection : public Object
{
	T_RTTI_CLASS;

public:
	Connection(
		Semaphore& connectionStringsLock,
		const std::map< std::wstring, std::wstring >& connectionStrings,
		net::StreamServer* streamServer,
		net::TcpSocket* clientSocket
	);

	void destroy();

	bool update();

	bool alive() const;

	void sendReply(const IMessage& message);

	uint32_t putObject(Object* object);

	Ref< Object > getObject(uint32_t handle);

	void releaseObject(uint32_t handle);

	void setDatabase(IProviderDatabase* database);

	IProviderDatabase* getDatabase() const;

	net::StreamServer* getStreamServer() const;

	template < typename ObjectType >
	Ref< ObjectType > getObject(uint32_t handle)
	{
		return dynamic_type_cast< ObjectType* >(getObject(handle));
	}

private:
	Thread* m_thread;
	Ref< net::StreamServer > m_streamServer;
	Ref< net::TcpSocket > m_clientSocket;
	Ref< net::BidirectionalObjectTransport > m_transport;
	RefArray< IMessageListener > m_messageListeners;
	std::map< uint32_t, Ref< Object > > m_objectStore;
	uint32_t m_nextHandle;
	Ref< IProviderDatabase > m_database;

	void messageThread();
};

	}
}

#endif	// traktor_db_Connection_H
