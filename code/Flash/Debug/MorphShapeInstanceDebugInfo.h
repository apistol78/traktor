/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_MorphShapeInstanceDebugInfo_H
#define traktor_flash_MorphShapeInstanceDebugInfo_H

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

class FlashMorphShapeInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS MorphShapeInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	MorphShapeInstanceDebugInfo();

	MorphShapeInstanceDebugInfo(const FlashMorphShapeInstance* instance, bool mask);

	bool getMask() const { return m_mask; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	bool m_mask;
};
	
	}
}

#endif	// traktor_flash_MorphShapeInstanceDebugInfo_H

