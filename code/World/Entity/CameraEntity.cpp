#include "World/Entity/CameraEntity.h"
#include "World/Entity/CameraEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.CameraEntity", CameraEntity, Entity)

CameraEntity::CameraEntity(const CameraEntityData* cameraData)
:	m_type(cameraData->getCameraType())
,	m_fov(cameraData->getFieldOfView())
,	m_width(cameraData->getWidth())
,	m_height(cameraData->getHeight())
,	m_transform(cameraData->getTransform())
{
}

void CameraEntity::update(const UpdateParams& update)
{
	m_transform.step();
}

void CameraEntity::setTransform(const Transform& transform)
{
	m_transform.set(transform);
}

bool CameraEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform.get();
	return true;
}

Aabb3 CameraEntity::getBoundingBox() const
{
	return Aabb3(
		Vector4::zero(),
		Vector4::zero()
	);
}

void CameraEntity::setCameraType(CameraType type)
{
	m_type = type;
}

CameraType CameraEntity::getCameraType() const
{
	return m_type;
}

void CameraEntity::setFieldOfView(float fov)
{
	m_fov = fov;
}

float CameraEntity::getFieldOfView() const
{
	return m_fov;
}

void CameraEntity::setWidth(float width)
{
	m_width = width;
}

float CameraEntity::getWidth() const
{
	return m_width;
}

void CameraEntity::setHeight(float height)
{
	m_height = height;
}

float CameraEntity::getHeight() const
{
	return m_height;
}

Transform CameraEntity::getTransform(float interval) const
{
	return m_transform.get(interval);
}

	}
}
