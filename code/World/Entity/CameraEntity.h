#ifndef traktor_world_CameraEntity_H
#define traktor_world_CameraEntity_H

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

class CameraEntityData;

/*! \brief Camera entity.
 * \ingroup World
 */
class T_DLLCLASS CameraEntity : public Entity
{
	T_RTTI_CLASS;

public:
	CameraEntity(const CameraEntityData* cameraData);

	virtual void update(const UpdateParams& update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	void setCameraType(CameraType type);

	CameraType getCameraType() const;

	void setFieldOfView(float fov);

	float getFieldOfView() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

	Transform getTransform(float interval) const;

private:
	CameraType m_type;
	float m_fov;
	float m_width;
	float m_height;
	IntervalTransform m_transform;
};

	}
}

#endif	// traktor_world_CameraEntity_H
