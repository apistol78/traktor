/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ShapeInstanceDebugInfo_H
#define traktor_flash_ShapeInstanceDebugInfo_H

#include "Flash/Debug/InstanceDebugInfo.h"

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

class FlashShapeInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS ShapeInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	ShapeInstanceDebugInfo();

	ShapeInstanceDebugInfo(const FlashShapeInstance* instance);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};
	
	}
}

#endif	// traktor_flash_ShapeInstanceDebugInfo_H

