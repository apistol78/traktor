/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_BitmapFilter_H
#define traktor_flash_BitmapFilter_H

#include "Flash/Action/ActionObjectRelay.h"

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
class T_DLLCLASS BitmapFilter : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	virtual Ref< BitmapFilter > clone() = 0;

protected:
	BitmapFilter(const char* const prototype);
};

	}
}

#endif	// traktor_flash_BitmapFilter_H
