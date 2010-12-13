#ifndef traktor_world_TransientEntity_H
#define traktor_world_TransientEntity_H

#include "World/Entity/SpatialEntity.h"

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
class T_DLLCLASS TransientEntity : public SpatialEntity
{
	T_RTTI_CLASS;

public:
	TransientEntity(
		GroupEntity* effectGroup,
		SpatialEntity* otherEntity,
		float duration
	);

	void render(
		WorldContext* worldContext,
		WorldRenderView* worldRenderView
	);

	virtual void update(const EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

private:
	Ref< GroupEntity > m_parentGroup;
	Ref< SpatialEntity > m_otherEntity;
	float m_duration;
};

	}
}

#endif	// traktor_world_TransientEntity_H
