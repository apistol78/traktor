#ifndef traktor_net_BidirectionalObjectTransport_H
#define traktor_net_BidirectionalObjectTransport_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"

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

class TcpSocket;

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS BidirectionalObjectTransport : public Object
{
	T_RTTI_CLASS;

public:
	enum Result
	{
		RtTimeout = 0,
		RtSuccess = 1,
		RtDisconnected = -1
	};

	BidirectionalObjectTransport(TcpSocket* socket);

	void close();

	bool send(const ISerializable* object);

	bool wait(int32_t timeout);

	Result recv(const TypeInfo& objectType, int32_t timeout, Ref< ISerializable >& outObject);

	void flush(const TypeInfo& objectType);

	bool connected() const { return m_socket != 0; }

	TcpSocket* getSocket() const { return m_socket; }

	template < typename ObjectType >
	Result recv(int32_t timeout, Ref< ObjectType >& outObject)
	{
		return recv(type_of< ObjectType >(), timeout, (Ref< ISerializable >&)outObject);
	}

	template < typename ObjectType >
	void flush()
	{
		flush(type_of< ObjectType >());
	}

private:
	Ref< TcpSocket > m_socket;
	RefArray< ISerializable > m_inQueue;
	AutoArrayPtr< uint8_t > m_buffer;
};

	}
}

#endif	// traktor_net_BidirectionalObjectTransport_H
