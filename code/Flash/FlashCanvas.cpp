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
	m_paths.clear();
	m_bounds = Aabb2();
	m_fillStyles.resize(0);
	m_lineStyles.resize(0);
	++m_dirtyTag;
}

void FlashCanvas::beginFill(const FlashFillStyle& fillStyle)
{
	m_fillStyles.push_back(fillStyle);
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

void FlashCanvas::moveTo(avm_number_t x, avm_number_t y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.moveTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void FlashCanvas::lineTo(avm_number_t x, avm_number_t y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.lineTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void FlashCanvas::curveTo(avm_number_t controlX, avm_number_t controlY, avm_number_t anchorX, avm_number_t anchorY)
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
