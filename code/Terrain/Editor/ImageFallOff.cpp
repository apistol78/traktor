#include "Drawing/Image.h"
#include "Terrain/Editor/ImageFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ImageFallOff", ImageFallOff, IFallOff)

ImageFallOff::ImageFallOff(const drawing::Image* image)
:	m_image(image)
{
}

float ImageFallOff::evaluate(float x, float y) const
{
	int32_t ix = int32_t((x * 0.5f + 0.5f) * (m_image->getWidth() - 1));
	int32_t iy = int32_t((y * 0.5f + 0.5f) * (m_image->getHeight() - 1));

	Color4f c;
	if (!m_image->getPixel(ix, iy, c))
		return 0.0f;

	return c.getRed();
}

	}
}
