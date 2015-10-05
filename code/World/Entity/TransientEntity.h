#ifndef traktor_world_TransientEntity_H
#define traktor_world_TransientEntity_H

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

class GroupEntity;
class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

/*! \brief Transient entity.
 * \ingroup World
 *
 * An transient entity is actually just a container for some
 * other "visual" entity. It's only responsibility is to
 * remove itself from parent group as soon as some
 * time has expired.
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

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

private:
	Ref< GroupEntity > m_parentGroup;
	Ref< Entity > m_otherEntity;
	float m_duration;
};

	}
}

#endif	// traktor_world_TransientEntity_H
