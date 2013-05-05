#ifndef traktor_ai_NavMeshResource_H
#define traktor_ai_NavMeshResource_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ai
	{

class T_DLLCLASS NavMeshResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_ai_NavMeshResource_H
