#ifndef traktor_net_IReplicatorStateListener_H
#define traktor_net_IReplicatorStateListener_H

#include "Core/Object.h"

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

class Replicator;
class ReplicatorProxy;

class T_DLLCLASS IReplicatorStateListener : public Object
{
	T_RTTI_CLASS;

public:
	enum
	{
		ReConnected = 1,
		ReDisconnected = 2,
		ReState = 3,
		ReStatus = 4
	};

	virtual void notify(
		Replicator* replicator,
		float eventTime,
		uint32_t eventId,
		ReplicatorProxy* proxy,
		const Object* eventObject
	) = 0;
};

	}
}

#endif	// traktor_net_IReplicatorStateListener_H
