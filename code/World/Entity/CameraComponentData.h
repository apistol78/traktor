#ifndef traktor_world_CameraComponentData_H
#define traktor_world_CameraComponentData_H

#include "World/IEntityComponentData.h"
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

/*! \brief Camera component data.
 * \ingroup World
 */
class T_DLLCLASS CameraComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	CameraComponentData();

	void setCameraType(CameraType type);

	CameraType getCameraType() const;

	void setFieldOfView(float fov);

	float getFieldOfView() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	CameraType m_type;
	float m_fov;
	float m_width;
	float m_height;
};

	}
}

#endif	// traktor_world_CameraComponentData_H
