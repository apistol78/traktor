#ifndef traktor_net_IValue_H
#define traktor_net_IValue_H

#include "Core/Rtti/ITypedObject.h"

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

class T_DLLCLASS IValue : public ITypedObject
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_net_IValue_H
