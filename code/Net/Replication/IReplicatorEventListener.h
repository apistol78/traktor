#pragma once

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
	virtual bool notify(
		Replicator* replicator,
		float eventTime,
		ReplicatorProxy* fromProxy,
		const Object* eventObject
	) = 0;
};

	}
}

