#ifndef traktor_world_VolumeEntity_H
#define traktor_world_VolumeEntity_H

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

class VolumeEntityData;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS VolumeEntity : public Entity
{
	T_RTTI_CLASS;

public:
	VolumeEntity(const VolumeEntityData* data);

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	bool inside(const Vector4& point) const;

private:
	Ref< const VolumeEntityData > m_data;
	Transform m_transform;
	Aabb3 m_boundingBox;
};

	}
}

#endif	// traktor_world_VolumeEntity_H
