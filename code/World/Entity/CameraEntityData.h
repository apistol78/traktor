#ifndef traktor_world_CameraEntityData_H
#define traktor_world_CameraEntityData_H

#include "World/EntityData.h"
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

/*! \brief Camera entity data.
 * \ingroup World
 */
class T_DLLCLASS CameraEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	CameraEntityData();

	void setCameraType(CameraType type);

	CameraType getCameraType() const;

	void setFieldOfView(float fov);

	float getFieldOfView() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

	virtual void serialize(ISerializer& s);

private:
	CameraType m_type;
	float m_fov;
	float m_width;
	float m_height;
};

	}
}

#endif	// traktor_world_CameraEntityData_H
