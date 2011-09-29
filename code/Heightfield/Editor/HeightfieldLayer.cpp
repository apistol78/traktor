#include <limits>
#include "Drawing/Image.h"
#include "Heightfield/Editor/HeightfieldLayer.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldLayer", HeightfieldLayer, Object)

Ref< HeightfieldLayer > HeightfieldLayer::createFromImage(drawing::Image* image)
{
	uint32_t size = image->getWidth();
	if (size != image->getHeight())
		return 0;

	Ref< HeightfieldLayer > layer = new HeightfieldLayer();
	layer->m_size = size;
	layer->m_heights.reset(new height_t [size * size]);

	for (uint32_t y = 0; y < size; ++y)
	{
		for (uint32_t x = 0; x < size; ++x)
		{
			Color4f imagePixel;
			image->getPixelUnsafe(x, y, imagePixel);
			layer->m_heights[x + y * size] = height_t(imagePixel.getRed() * std::numeric_limits< height_t >::max());
		}
	}

	return layer;
}

Ref< HeightfieldLayer > HeightfieldLayer::createEmpty(uint32_t size, height_t height)
{
	Ref< HeightfieldLayer > layer = new HeightfieldLayer();
	layer->m_size = size;
	layer->m_heights.reset(new height_t [size * size]);

	for (uint32_t i = 0; i < size * size; ++i)
		layer->m_heights[i] = height;

	return layer;
}

HeightfieldLayer::HeightfieldLayer()
:	m_size(0)
{
}

	}
}
