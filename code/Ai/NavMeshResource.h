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

/*! \brief Navigation mesh persistent resource.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_ai_NavMeshResource_H
