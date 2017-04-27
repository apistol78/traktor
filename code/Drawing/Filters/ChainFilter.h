/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_ChainFilter_H
#define traktor_drawing_ChainFilter_H

#include "Core/RefArray.h"
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

/*! \brief Chain of filters.
 * \ingroup Drawing
 */
class T_DLLCLASS ChainFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	void add(IImageFilter* filter);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	RefArray< IImageFilter > m_filters;
};
	
	}
}

#endif	// traktor_drawing_ChainFilter_H
