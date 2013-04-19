#include "Drawing/Image.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Classes/BitmapData.h"

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
