#include "Flash/FlashBitmap.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashTypes.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCanvas", FlashCanvas, Object)

FlashCanvas::FlashCanvas()
:	m_cacheTag(allocateCacheTag())
,	m_dirtyTag(0)
,	m_drawing(false)
{
	clear();
}

int32_t FlashCanvas::getCacheTag() const
{
	return m_cacheTag;
}

int32_t FlashCanvas::getDirtyTag() const
{
	return m_dirtyTag;
}

void FlashCanvas::clear()
{
	m_dictionary = FlashDictionary();
	m_paths.clear();
	m_bounds = Aabb2();
	m_fillStyles.resize(0);
	m_lineStyles.resize(0);
	++m_dirtyTag;
}

void FlashCanvas::beginFill(const Color4f& color)
{
	FlashFillStyle style;
	style.create(color);
	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void FlashCanvas::beginGradientFill(FlashFillStyle::GradientType gradientType, const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords, const Matrix33& gradientMatrix)
{
	FlashFillStyle style;
	style.create(gradientType, colorRecords, gradientMatrix);
	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void FlashCanvas::beginBitmapFill(FlashBitmap* image, const Matrix33& bitmapMatrix, bool repeat)
{
	uint16_t bitmapId = m_dictionary.addBitmap(image);

	FlashFillStyle style;
	style.create(bitmapId, bitmapMatrix, repeat);

	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void FlashCanvas::endFill()
{
	if (!m_drawing)
		return;

	uint16_t fillStyle = uint16_t(m_fillStyles.size());

	Path& p = m_paths.back();
	p.end(fillStyle, 0, 0);

	m_drawing = false;
	++m_dirtyTag;
}

void FlashCanvas::moveTo(float x, float y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.moveTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void FlashCanvas::lineTo(float x, float y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.lineTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void FlashCanvas::curveTo(float controlX, float controlY, float anchorX, float anchorY)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.quadraticTo(int32_t(controlX), int32_t(controlY), int32_t(anchorX), int32_t(anchorY), Path::CmAbsolute);
	m_bounds.contain(Vector2(controlX, controlY));
	m_bounds.contain(Vector2(anchorX, anchorY));
}

	}
}
