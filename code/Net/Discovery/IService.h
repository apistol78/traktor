#ifndef traktor_net_IService_H
#define traktor_net_IService_H

#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS IService : public Serializable
{
	T_RTTI_CLASS(IService)
};

	}
}

#endif	// traktor_net_IService_H
