#include "Drawing/Image.h"
#include "Drawing/Filters/ChainFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ChainFilter", ChainFilter, IImageFilter)

void ChainFilter::add(IImageFilter* filter)
{
	m_filters.push_back(filter);
}

void ChainFilter::apply(Image* image) const
{
	for (auto filter : m_filters)
		image->apply(filter);
}

	}
}
