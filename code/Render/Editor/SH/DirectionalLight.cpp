#include "Render/Editor/SH/DirectionalLight.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"DirectionalLight", DirectionalLight, SHFunction)

DirectionalLight::DirectionalLight(const Vector4& direction)
:	m_direction(direction)
{
}

Vector4 DirectionalLight::evaluate(float phi, float theta, const Vector4& unit) const
{
	return Vector4(1.0f, 1.0f, 1.0f, 0.0f) * clamp(dot3(unit, m_direction), Scalar(0.0f), Scalar(1.0f));
}

	}
}
