#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

}

namespace traktor::world
{

class EntityData;

/*! Component data interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityComponentData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void setTransform(const EntityData* owner, const Transform& transform) = 0;
};

}
