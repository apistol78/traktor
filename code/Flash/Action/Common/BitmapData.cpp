#include "Drawing/Image.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/BitmapData.h"

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

int32_t BitmapData::getWidth() const
{
	return m_image->getWidth();
}

int32_t BitmapData::getHeight() const
{
	return m_image->getHeight();
}

drawing::Image* BitmapData::getImage() const
{
	return m_image;
}

	}
}
