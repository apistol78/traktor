#pragma once

#include "Core/RefArray.h"
#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Chain of filters.
 * \ingroup Drawing
 */
class T_DLLCLASS ChainFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	void add(IImageFilter* filter);

protected:
	virtual void apply(Image* image) const override final;

private:
	RefArray< IImageFilter > m_filters;
};

}
