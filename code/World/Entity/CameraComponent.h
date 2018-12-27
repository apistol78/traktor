/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_CameraComponent_H
#define traktor_world_CameraComponent_H

#include "World/IEntityComponent.h"

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

class CameraComponentData;

/*! \brief Camera component.
 * \ingroup World
 */
class T_DLLCLASS CameraComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	CameraComponent(const CameraComponentData* cameraData);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	void setCameraType(CameraType type);

	CameraType getCameraType() const;

	void setFieldOfView(float fov);

	float getFieldOfView() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

private:
	CameraType m_type;
	float m_fov;
	float m_width;
	float m_height;
};

	}
}

#endif	// traktor_world_CameraComponent_H
