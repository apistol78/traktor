#ifndef traktor_world_ComponentEntity_H
#define traktor_world_ComponentEntity_H

#include "Core/Containers/SmallMap.h"
#include "Core/Math/IntervalTransform.h"
#include "World/Entity.h"

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

class IEntityComponent;
class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS ComponentEntity : public Entity
{
	T_RTTI_CLASS;

public:
	ComponentEntity();

	virtual void destroy() T_OVERRIDE;

	virtual void setTransform(const Transform& transform) T_OVERRIDE;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE;

	virtual void update(const UpdateParams& update) T_OVERRIDE;

	/*! \brief Set component in character instance.
	 */
	void setComponent(IEntityComponent* component);

	/*! \brief Get component of type.
	 */
	IEntityComponent* getComponent(const TypeInfo& componentType) const;

	void render(WorldContext& worldContext, WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass);

private:
	friend class WorldEntityFactory;

	Transform m_transform;
	SmallMap< const TypeInfo*, Ref< IEntityComponent > > m_components;
};

	}
}

#endif	// traktor_world_ComponentEntity_H
