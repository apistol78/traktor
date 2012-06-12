#include <limits>
#include "Flash/FlashCanvas.h"
#include "Flash/FlashTypes.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

void expandBounds(SwfRect& bounds, avm_number_t x, avm_number_t y)
{
	bounds.min.x = std::min< float >(bounds.min.x, x);
	bounds.min.y = std::min< float >(bounds.min.y, y);
	bounds.max.x = std::max< float >(bounds.max.x, x);
	bounds.max.y = std::max< float >(bounds.max.y, y);
}

		}

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
	m_bounds.min = Vector2( std::numeric_limits< float >::max(),  std::numeric_limits< float >::max());
	m_bounds.max = Vector2(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max());
	m_fillStyles.clear();
	m_lineStyles.clear();
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
	p.moveTo(x, y, Path::CmAbsolute);

	expandBounds(m_bounds, x, y);
}

void FlashCanvas::lineTo(avm_number_t x, avm_number_t y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.lineTo(x, y, Path::CmAbsolute);

	expandBounds(m_bounds, x, y);
}

void FlashCanvas::curveTo(avm_number_t controlX, avm_number_t controlY, avm_number_t anchorX, avm_number_t anchorY)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.quadraticTo(controlX, controlY, anchorX, anchorY, Path::CmAbsolute);

	expandBounds(m_bounds, controlX, controlY);
	expandBounds(m_bounds, anchorX, anchorY);
}

	}
}
