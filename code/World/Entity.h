/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_Entity_H
#define traktor_world_Entity_H

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
#include "World/WorldTypes.h"

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

/*! \brief World entity base class.
 * \ingroup World
 */
class T_DLLCLASS Entity : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~Entity();

	/*! \brief Destroy entity resources.
	 *
	 * Called automatically from Entity destructor
	 * in order to destroy any resources allocated
	 * by the entity.
	 *
	 * \note This may be called multiple times for
	 * a single entity so care must be taken when
	 * performing the destruction.
	 */
	virtual void destroy();

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
	virtual Aabb3 getBoundingBox() const = 0;

	/*! \brief Get entity bounding box.
	 * Return entity bounding box in world space.
	 *
	 * \return Entity bounding box.
	 */
	virtual Aabb3 getWorldBoundingBox() const;

	/*! \brief Update entity.
	 *
	 * \param update Update parameters.
	 */
	virtual void update(const UpdateParams& update) = 0;
};

	}
}

#endif	// traktor_world_Entity_H
