#ifndef traktor_db_Connection_H
#define traktor_db_Connection_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_SERVER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace net
	{

class TcpSocket;

	}

	namespace db
	{

class Configuration;
class MessageTransport;
class IMessage;
class IMessageListener;
class IProviderDatabase;

/*! \brief Client connection.
 * \ingroup Database
 */
class T_DLLCLASS Connection : public Object
{
	T_RTTI_CLASS(Connection)

public:
	Connection(const Configuration* configuration, net::TcpSocket* clientSocket);

	void destroy();

	bool update();

	bool alive() const;

	void sendReply(const IMessage& message);

	uint32_t putObject(Object* object);

	Object* getObject(uint32_t handle);

	void releaseObject(uint32_t handle);

	void setDatabase(IProviderDatabase* database);

	IProviderDatabase* getDatabase() const;

	template < typename ObjectType >
	ObjectType* getObject(uint32_t handle)
	{
		return dynamic_type_cast< ObjectType* >(getObject(handle));
	}

private:
	Thread* m_thread;
	Ref< const Configuration > m_configuration;
	Ref< net::TcpSocket > m_clientSocket;
	Ref< MessageTransport > m_messageTransport;
	RefArray< IMessageListener > m_messageListeners;
	std::map< uint32_t, Ref< Object > > m_objectStore;
	uint32_t m_nextHandle;
	Ref< IProviderDatabase > m_database;

	void messageThread();
};

	}
}

#endif	// traktor_db_Connection_H
