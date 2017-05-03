/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_SpriteInstanceDebugInfo_H
#define traktor_flash_SpriteInstanceDebugInfo_H

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

class FlashSpriteInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS SpriteInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	SpriteInstanceDebugInfo();

	SpriteInstanceDebugInfo(const FlashSpriteInstance* instance);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};
	
	}
}

#endif	// traktor_flash_SpriteInstanceDebugInfo_H
