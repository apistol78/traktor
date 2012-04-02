#ifndef traktor_world_NullEntity_H
#define traktor_world_NullEntity_H

#include "Core/Math/IntervalTransform.h"
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

/*! \brief Null entity.
 * \ingroup World
 */
class T_DLLCLASS NullEntity : public Entity
{
	T_RTTI_CLASS;

public:
	NullEntity(const Transform& transform);

	virtual void update(const UpdateParams& update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	Transform getTransform(float interval) const;

private:
	IntervalTransform m_transform;
};

	}
}

#endif	// traktor_world_NullEntity_H
