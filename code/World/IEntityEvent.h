#ifndef traktor_world_IEntityEvent_H
#define traktor_world_IEntityEvent_H

#include "Core/Object.h"

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

	namespace world
	{

class Entity;
class IEntityEventInstance;
class IEntityEventManager;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS IEntityEvent : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< IEntityEventInstance > createInstance(IEntityEventManager* eventManager, Entity* sender, const Transform& Toffset) const = 0;
};

	}
}

#endif	// traktor_world_IEntityEvent_H
