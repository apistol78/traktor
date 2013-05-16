#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"

namespace traktor
{
	namespace scene
	{

Camera::Camera()
:	m_enable(false)
{
	const Quaternion qx = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), deg2rad(45.0f));
	const Quaternion qy = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), deg2rad(45.0f));

	m_position = Vector4(-4.0f, 4.0f, -4.0f, 1.0f);
	m_orientation = qx * qy;
}

void Camera::setEnable(bool enable)
{
	m_enable = enable;
}

bool Camera::isEnable() const
{
	return m_enable;
}

void Camera::place(const Vector4& position)
{
	m_position = position;
}

void Camera::move(const Vector4& direction)
{
	m_position += m_orientation * direction;
}

void Camera::rotate(float dy, float dx)
{
	Quaternion qx = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), -dx);
	Quaternion qy = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), -dy);

	m_orientation = qx * m_orientation * qy;
	m_orientation = m_orientation.normalized();
}

void Camera::setFollowEntity(EntityAdapter* followEntity)
{
	m_followEntity = followEntity;
}

void Camera::setLookAtEntity(EntityAdapter* lookAtEntity)
{
	m_lookAtEntity = lookAtEntity;
}

Matrix44 Camera::getWorld() const
{
	Matrix44 world = translate(m_position) * m_orientation.toMatrix44();
	if (m_followEntity || m_lookAtEntity)
	{
		if (m_followEntity)
			world = m_followEntity->getTransform().toMatrix44();

		if (m_lookAtEntity)
		{
			Vector4 origin = world.translation().xyz1();
			Vector4 target = m_lookAtEntity->getTransform().translation().xyz1();

			world = lookAt(
				origin,
				target,
				m_followEntity ? world.axisY() : Vector4(0.0f, 1.0f, 0.0f, 0.0f)
			).inverse();
		}
	}
	return world;
}

Matrix44 Camera::getView() const
{
	return getWorld().inverse();
}

	}
}
