#include "Core/Math/Vector2.h"
#include "Drawing/Image.h"
#include "Shape/Editor/Bake/IblProbe.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

// \tbd Need to verify to CubeMap.cpp
Vector2 toEquirectangular(const Vector4& direction)
{
	float theta = std::acos(direction.y());
	float phi = std::atan2(direction.z(), direction.x());
	return Vector2(phi, theta);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.IblProbe", IblProbe, IProbe)

IblProbe::IblProbe(const drawing::Image* radiance, const drawing::Image* importance)
:	m_radiance(radiance)
,	m_importance(importance)
{
}

float IblProbe::getDensity(const Vector4& direction) const
{
	Vector2 e = toEquirectangular(direction);

	float x = e.x / TWO_PI;
	float y = e.y / PI;

	int32_t u = (int32_t)(x * m_importance->getWidth());
	int32_t v = (int32_t)(y * m_importance->getHeight());

	Color4f cl;
	m_importance->getPixel(u, v, cl);
	return cl.getRed();
}

float IblProbe::getProbability(const Vector4& direction) const
{
	Vector2 e = toEquirectangular(direction);

	float x = e.x / TWO_PI;
	float y = e.y / PI;

	int32_t u = (int32_t)(x * m_importance->getWidth());
	int32_t v = (int32_t)(y * m_importance->getHeight());

	Color4f cl;
	m_importance->getPixel(u, v, cl);
	return cl.getGreen();
}

Color4f IblProbe::sample(const Vector4& direction) const
{
	Vector2 e = toEquirectangular(direction);

	float x = e.x / TWO_PI;
	float y = e.y / PI;

	int32_t u = (int32_t)(x * m_radiance->getWidth());
	int32_t v = (int32_t)(y * m_radiance->getHeight());

	Color4f cl;
	m_radiance->getPixel(u, v, cl);
	return cl;
}

	}
}
