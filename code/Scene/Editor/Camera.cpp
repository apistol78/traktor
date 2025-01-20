/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/Camera.h"

#include "Core/Math/Const.h"

namespace traktor::scene
{

Camera::Camera()
	: m_enable(false)
{
	const Quaternion qx = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), deg2rad(45.0f));
	const Quaternion qy = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), deg2rad(45.0f));

	m_position = Vector4(-4.0f, 4.0f, -4.0f, 1.0f);
	m_orientation = qx * qy;

	m_filteredPosition = m_position;
	m_filteredOrientation = m_orientation;
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
	m_filteredPosition = position;
}

void Camera::move(const Vector4& direction)
{
	m_position += m_orientation * direction;
}

void Camera::rotate(float dy, float dx)
{
	const Quaternion qx = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), -dx);
	const Quaternion qy = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), -dy);

	m_orientation = qx * m_orientation * qy;
	m_orientation = m_orientation.normalized();
}

bool Camera::update(float deltaTime)
{
	const Scalar k = clamp(Scalar(deltaTime) * 6.0_simd, 0.0_simd, 1.0_simd);

	m_filteredPosition = lerp(m_filteredPosition, m_position, k);
	m_filteredOrientation = lerp(m_filteredOrientation, m_orientation, k);

	// Check if we are close enough, we are given some slack due to auto update will redraw a couple of more frames.
	if ((m_filteredPosition - m_position).length() < 0.01_simd && (m_filteredOrientation * m_orientation.inverse()).toEulerAngles().length() < 0.01_simd)
		return true;

	return false;
}

Transform Camera::getWorld() const
{
	return Transform(m_filteredPosition) * Transform(m_filteredOrientation);
}

Transform Camera::getView() const
{
	return getWorld().inverse();
}

}
