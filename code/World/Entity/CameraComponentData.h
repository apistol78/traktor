#pragma once

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

/*! Camera component data.
 * \ingroup World
 */
class T_DLLCLASS CameraComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	CameraComponentData();

	void setProjection(Projection projection);

	Projection getProjection() const;

	void setFieldOfView(float fov);

	float getFieldOfView() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Projection m_projection;
	float m_fov;
	float m_width;
	float m_height;
};

	}
}

