/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_BidirectionalObjectTransport_H
#define traktor_net_BidirectionalObjectTransport_H

#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"

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

	virtual ~BidirectionalObjectTransport();

	void close();

	bool send(const ISerializable* object);

	Result recv(const TypeInfoSet& objectTypes, int32_t timeout, Ref< ISerializable >& outObject);

	void flush(const TypeInfo& objectType);

	bool connected() const { return m_socket != 0; }

	TcpSocket* getSocket() const { return m_socket; }

	template < typename ObjectType >
	Result recv(int32_t timeout, Ref< ObjectType >& outObject)
	{
		Ref< ISerializable > obj;
		Result result;
		
		result = recv(makeTypeInfoSet< ObjectType >(), timeout, obj);
		if (result != RtSuccess)
			return result;

		outObject = dynamic_type_cast< ObjectType* >(obj);
		return RtSuccess;
	}

	template < typename ObjectType >
	void flush()
	{
		flush(type_of< ObjectType >());
	}

private:
	Ref< TcpSocket > m_socket;
	SmallMap< const TypeInfo*, RefArray< ISerializable > > m_inQueue;
	std::vector< uint8_t* > m_buffers;
	ThreadLocal m_threadBuffer;
	Semaphore m_lock;
};

	}
}

#endif	// traktor_net_BidirectionalObjectTransport_H
