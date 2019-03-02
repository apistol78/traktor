#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Render/Editor/SH/IBLProbe.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"IBLProbe", IBLProbe, SHFunction)

IBLProbe::IBLProbe(drawing::Image* image, const Vector4& modulate)
:	m_image(image)
,	m_modulate(modulate)
{
}

float IBLProbe::evaluate(float phi, float theta, const Vector4& unit) const
{
	float T_MATH_ALIGN16 u[4];
	unit.storeAligned(u);

	float r = float((1.0f / PI) * acos(u[2]) / sqrt(u[0] * u[0] + u[1] * u[1]));

	int x = int(m_image->getWidth() * (u[0] * r * 0.5f + 0.5f));
	int y = int(m_image->getHeight() * (-u[1] * r * 0.5f + 0.5f));

	if (x < 0 || y < 0)
		return 0.0f;

	Color4f color;
	if (!m_image->getPixel(x, y, color))
		return 0.0f;

	Scalar result = dot3(
		Vector4(color.getRed(), color.getGreen(), color.getBlue()),
		m_modulate
	);
	T_ASSERT (result >= 0.0f && result <= 1.0f);

	return result;
}

	}
}
