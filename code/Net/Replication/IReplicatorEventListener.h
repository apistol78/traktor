#ifndef traktor_net_IReplicatorEventListener_H
#define traktor_net_IReplicatorEventListener_H

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

class T_DLLCLASS IReplicatorEventListener : public Object
{
	T_RTTI_CLASS;

public:
	virtual void notify(
		Replicator* replicator,
		float eventTime,
		ReplicatorProxy* fromProxy,
		const Object* eventObject
	) = 0;
};

	}
}

#endif	// traktor_net_IReplicatorEventListener_H
