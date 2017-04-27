/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_SwizzleFilter_H
#define traktor_drawing_SwizzleFilter_H

#include <string>
#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief Swizzle color channels.
 * \ingroup Drawing
 */
class T_DLLCLASS SwizzleFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	SwizzleFilter(const std::wstring& swizzle);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	wchar_t m_swizzle[4];
};
	
	}
}

#endif	// traktor_drawing_SwizzleFilter_H
