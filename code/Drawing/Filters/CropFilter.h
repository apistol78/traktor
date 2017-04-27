/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_CropFilter_H
#define traktor_drawing_CropFilter_H

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
	
/*! \brief Crop filter.
 * \ingroup Drawing
 */
class T_DLLCLASS CropFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	enum AnchorType
	{
		AtLeft = -1,
		AtUp = -1,
		AtCenter = 0,
		AtRight = 1,
		AtDown = 1
	};

	CropFilter(
		AnchorType anchorX,
		AnchorType anchorY,
		int32_t width,
		int32_t height
	);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	AnchorType m_anchorX;
	AnchorType m_anchorY;
	int32_t m_width;
	int32_t m_height;
};
	
	}
}

#endif	// traktor_drawing_CropFilter_H
