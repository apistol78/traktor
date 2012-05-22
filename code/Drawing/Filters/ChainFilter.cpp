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

Ref< Image > ChainFilter::apply(const Image* image) const
{
	Ref< const Image > source = image;
	Ref< Image > target;

	for (RefArray< IImageFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
	{
		target = source->applyFilter(*i);
		source = target;
	}

	return target;
}

	}
}
