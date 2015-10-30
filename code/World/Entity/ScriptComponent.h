#ifndef traktor_world_ScriptComponent_H
#define traktor_world_ScriptComponent_H

#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace world
	{

class Entity;

/*! \brief Script component.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	ScriptComponent(Entity* owner, const resource::Proxy< IRuntimeClass >& clazz);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void render(WorldContext& worldContext, WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass, const Transform& transform) T_OVERRIDE T_FINAL;

private:
	Entity* m_owner;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< ITypedObject > m_object;
	uint32_t m_methodUpdate;
};

	}
}

#endif	// traktor_world_ScriptComponent_H
