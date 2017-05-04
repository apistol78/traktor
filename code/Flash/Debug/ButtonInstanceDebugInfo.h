/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ButtonInstanceDebugInfo_H
#define traktor_flash_ButtonInstanceDebugInfo_H

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

class FlashButtonInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS ButtonInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	ButtonInstanceDebugInfo();

	ButtonInstanceDebugInfo(const FlashButtonInstance* instance);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};
	
	}
}

#endif	// traktor_flash_ButtonInstanceDebugInfo_H

