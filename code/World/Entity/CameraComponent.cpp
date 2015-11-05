#include "World/Entity/CameraComponent.h"
#include "World/Entity/CameraComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.CameraComponent", CameraComponent, IEntityComponent)

CameraComponent::CameraComponent(const CameraComponentData* cameraData)
:	m_type(cameraData->getCameraType())
,	m_fov(cameraData->getFieldOfView())
,	m_width(cameraData->getWidth())
,	m_height(cameraData->getHeight())
{
}

void CameraComponent::destroy()
{
}

void CameraComponent::update(const UpdateParams& update)
{
}

void CameraComponent::setTransform(const Transform& transform)
{
}

Aabb3 CameraComponent::getBoundingBox() const
{
	return Aabb3();
}

void CameraComponent::setCameraType(CameraType type)
{
	m_type = type;
}

CameraType CameraComponent::getCameraType() const
{
	return m_type;
}

void CameraComponent::setFieldOfView(float fov)
{
	m_fov = fov;
}

float CameraComponent::getFieldOfView() const
{
	return m_fov;
}

void CameraComponent::setWidth(float width)
{
	m_width = width;
}

float CameraComponent::getWidth() const
{
	return m_width;
}

void CameraComponent::setHeight(float height)
{
	m_height = height;
}

float CameraComponent::getHeight() const
{
	return m_height;
}

	}
}
