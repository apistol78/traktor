#ifndef traktor_world_SpatialEntity_H
#define traktor_world_SpatialEntity_H

#include "World/Entity/Entity.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Aabb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Spatial entity.
 * \ingroup World
 */
class T_DLLCLASS SpatialEntity : public Entity
{
	T_RTTI_CLASS;

public:
	/*! \brief Set entity transform.
	 *
	 * \param transform Entity transform.
	 */
	virtual void setTransform(const Transform& transform);

	/*! \brief Get entity transform.
	 *
	 * \param outTransform Returning entity transform.
	 * \return True if entity have a transform.
	 */
	virtual bool getTransform(Transform& outTransform) const;

	/*! \brief Get entity bounding box.
	 * Return entity bounding box in entity space.
	 *
	 * \return Entity bounding box.
	 */
	virtual Aabb getBoundingBox() const = 0;

	/*! \brief Get entity bounding box.
	 * Return entity bounding box in world space.
	 *
	 * \return Entity bounding box.
	 */
	virtual Aabb getWorldBoundingBox() const;
};

	}
}

#endif	// traktor_world_SpatialEntity_H
