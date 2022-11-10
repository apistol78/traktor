#pragma once

#include <string>
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

/*! Swizzle color channels.
 * \ingroup Drawing
 */
class T_DLLCLASS SwizzleFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit SwizzleFilter(const std::wstring& swizzle);

protected:
	virtual void apply(Image* image) const override final;

private:
	wchar_t m_swizzle[4];
};

}
