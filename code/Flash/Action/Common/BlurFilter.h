/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_BlurFilter_H
#define traktor_flash_BlurFilter_H

#include "Flash/Action/ActionTypes.h"
#include "Flash/Action/Common/BitmapFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS BlurFilter : public BitmapFilter
{
	T_RTTI_CLASS;

public:
	BlurFilter(float blurX, float blurY, int32_t quality);

	virtual Ref< BitmapFilter > clone() T_OVERRIDE T_FINAL;

	float getBlurX();

	void setBlurX(float blurX);

	float getBlurY();

	void setBlurY(float blurY);

	int32_t getQuality();

	void setQuality(int32_t quality);

private:
	float m_blurX;
	float m_blurY;
	int32_t m_quality;
};

	}
}

#endif	// traktor_flash_BlurFilter_H
