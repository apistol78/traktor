#include "Scene/Editor/Camera.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_cameraSpeed = 3.0f;

inline void decompose(const Matrix44& transform, Vector4& outPosition, Quaternion& outOrientation)
{
	outPosition = transform.translation();
	outOrientation = Quaternion(transform);
}

		}

Camera::Camera(const Matrix44& transform)
{
	decompose(transform, m_target.position, m_target.orientation);
	m_current = m_target;
}

void Camera::move(const Vector4& direction)
{
	m_target.position += m_target.orientation.toMatrix44().inverseOrtho() * direction;
	m_current = m_target;
}

void Camera::rotate(float dy, float dx)
{
	m_target.orientation *= Quaternion(Vector4(0.0f, dx, 0.0f, 0.0f));
	m_target.orientation *= Quaternion(m_target.orientation.inverse() * Vector4(dy, 0.0f, 0.0f, 0.0f));
	m_target.orientation = m_target.orientation.normalized();
	m_current = m_target;
}

void Camera::setCurrentView(const Matrix44& transform)
{
	decompose(transform, m_current.position, m_current.orientation);
}

void Camera::setTargetView(const Matrix44& transform)
{
	decompose(transform, m_target.position, m_target.orientation);
}

void Camera::update(float deltaTime)
{
	if (m_current.position != m_target.position || m_current.orientation != m_target.orientation)
	{
		deltaTime = min< float >(deltaTime * c_cameraSpeed, 1.0f);
		m_current.position = lerp(m_current.position, m_target.position, Scalar(deltaTime));
		m_current.orientation = lerp(m_current.orientation, m_target.orientation, deltaTime);
	}
}

Matrix44 Camera::getCurrentView() const
{
	return translate(-m_current.position) * m_current.orientation.toMatrix44();
}

Matrix44 Camera::getTargetView() const
{
	return translate(-m_target.position) * m_target.orientation.toMatrix44();
}

	}
}
