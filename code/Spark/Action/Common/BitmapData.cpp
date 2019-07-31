#include "Drawing/Image.h"
#include "Drawing/Filters/BlurFilter.h"
#include "Spark/MovieRenderer.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Action/ActionValue.h"
#include "Spark/Action/Common/BitmapData.h"
#include "Spark/Action/Common/BlurFilter.h"
#include "Spark/Action/Common/Rectangle.h"
#include "Spark/Sw/SwDisplayRenderer.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.BitmapData", BitmapData, ActionObjectRelay)

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

void BitmapData::draw(SpriteInstance* source)
{
	SwDisplayRenderer displayRenderer(m_image, false);
	MovieRenderer movieRenderer(&displayRenderer, 0);

	Aabb2 frameBounds = source->getLocalBounds();
	Vector4 frameTransform(0.0f, 0.0f, 1.0f, 1.0f);

	float viewWidth = m_image->getWidth();
	float viewHeight = m_image->getHeight();

	movieRenderer.render(
		source,
		frameBounds,
		frameTransform,
		viewWidth,
		viewHeight
	);
}

bool BitmapData::save(const std::wstring& fileName)
{
	return m_image->save(fileName);
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
