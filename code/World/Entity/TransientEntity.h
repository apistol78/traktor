#ifndef traktor_world_TransientEntity_H
#define traktor_world_TransientEntity_H

#include "World/Entity/Entity.h"

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

class GroupEntity;
class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

/*! \brief Transient entity.
 * \ingroup World
 *
 * An transient entity is actually just a container for some
 * other "visual" entity. It's only responsibility is to
 * remove itself from parent group as soon as it
 * has expired.
 */
class T_DLLCLASS TransientEntity : public Entity
{
	T_RTTI_CLASS;

public:
	TransientEntity(
		GroupEntity* effectGroup,
		Entity* otherEntity,
		float duration
	);

	void precull(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView
	);

	void render(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		IWorldRenderPass& worldRenderPass
	);

	virtual void update(const UpdateParams& update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

private:
	Ref< GroupEntity > m_parentGroup;
	Ref< Entity > m_otherEntity;
	float m_duration;
};

	}
}

#endif	// traktor_world_TransientEntity_H
