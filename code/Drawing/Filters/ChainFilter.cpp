/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	for (RefArray< IImageFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		image->apply(*i);
}

	}
}
