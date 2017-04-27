/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Scene/Editor/Camera.h"

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

Transform Camera::getWorld() const
{
	return Transform(m_position) * Transform(m_orientation);
}

Transform Camera::getView() const
{
	return getWorld().inverse();
}

	}
}
