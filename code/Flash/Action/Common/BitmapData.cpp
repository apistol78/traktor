#include "Drawing/Image.h"
#include "Drawing/Filters/BlurFilter.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/BitmapData.h"
#include "Flash/Action/Common/BlurFilter.h"
#include "Flash/Action/Common/Rectangle.h"
#include "Flash/Sw/SwDisplayRenderer.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.BitmapData", BitmapData, ActionObjectRelay)

BitmapData::BitmapData(int32_t width, int32_t height, bool transparent, uint32_t fillColor)
:	ActionObjectRelay("flash.display.BitmapData")
{
	m_image = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		width,
		height
	);

	m_image->clear(Color4f(
		((fillColor & 0x00ff0000) >> 16) / 255.0f,
		((fillColor & 0x0000ff00) >>  8) / 255.0f,
		((fillColor & 0x000000ff) >>  0) / 255.0f,
		transparent ? ((fillColor & 0xff000000) >> 24) / 255.0f : 1.0f
	));
}

BitmapData::BitmapData(drawing::Image* image)
:	ActionObjectRelay("flash.display.BitmapData")
,	m_image(image)
{
}

int32_t BitmapData::getWidth()
{
	return m_image->getWidth();
}

int32_t BitmapData::getHeight()
{
	return m_image->getHeight();
}

bool BitmapData::getTransparent()
{
	return true;
}

Ref< Rectangle > BitmapData::getRectangle()
{
	return new Rectangle();
}

void BitmapData::applyFilter(const BitmapData* sourceBitmapData, const Rectangle* sourceRect, const Point* destPoint, const BitmapFilter* filter)
{
	if (BlurFilter* blurFilter = dynamic_type_cast< BlurFilter* >(const_cast< BitmapFilter* >(filter)))
	{
		drawing::BlurFilter f(blurFilter->getBlurX(), blurFilter->getBlurY());
		m_image->apply(&f);
	}
}

void BitmapData::draw(FlashSpriteInstance* source)
{
	SwDisplayRenderer displayRenderer(m_image, false);
	FlashMovieRenderer movieRenderer(&displayRenderer);

	Aabb2 frameBounds = source->getLocalBounds();
	Vector4 frameTransform(0.0f, 0.0f, 1.0f, 1.0f);

	float viewWidth = m_image->getWidth();
	float viewHeight = m_image->getHeight();

	movieRenderer.renderFrame(
		source,
		frameBounds,
		frameTransform,
		viewWidth,
		viewHeight
	);
}

void BitmapData::fillRect(const Rectangle* rectangle, uint32_t color)
{
}

drawing::Image* BitmapData::getImage() const
{
	return m_image;
}

	}
}
