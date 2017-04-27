/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_SharpenFilter_H
#define traktor_drawing_SharpenFilter_H

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
	
/*! \brief Sharpen filter.
 * \ingroup Drawing
 */
class T_DLLCLASS SharpenFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	SharpenFilter(int radius, float strength);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	int m_radius;
	float m_strength;
};
	
	}
}

#endif	// traktor_drawing_SharpenFilter_H
