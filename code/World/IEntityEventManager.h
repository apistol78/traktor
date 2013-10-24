#ifndef traktor_world_IEntityEventManager_H
#define traktor_world_IEntityEventManager_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class EntityEventSet;
class IEntityEvent;
class IWorldRenderer;
struct UpdateParams;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS IEntityEventManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual void raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset = Transform()) = 0;

	virtual void raise(const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset = Transform()) = 0;

	virtual void update(const UpdateParams& update) = 0;

	virtual void build(IWorldRenderer* worldRenderer) = 0;
};

	}
}

#endif	// traktor_world_IEntityEventManager_H
