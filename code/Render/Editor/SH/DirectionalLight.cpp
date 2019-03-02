#include "Render/Editor/SH/DirectionalLight.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"DirectionalLight", DirectionalLight, SHFunction)

DirectionalLight::DirectionalLight(const Vector4& direction, float cutOffAngle)
:	m_direction(direction)
,	m_cutOffAngle(cutOffAngle)
{
}

float DirectionalLight::evaluate(float phi, float theta, const Vector4& unit) const
{
	Scalar cosPhi = dot3(unit, m_direction);
	return acos(cosPhi) <= m_cutOffAngle ? 1.0f : 0.0f;
}

	}
}
