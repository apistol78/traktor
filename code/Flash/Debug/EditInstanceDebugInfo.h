/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_EditInstanceDebugInfo_H
#define traktor_flash_EditInstanceDebugInfo_H

#include "Flash/SwfTypes.h"
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
	
class FlashEditInstance;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS EditInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	EditInstanceDebugInfo();

	EditInstanceDebugInfo(const FlashEditInstance* instance);

	const std::wstring& getText() const { return m_text; }

	SwfTextAlignType getTextAlign() const { return m_textAlign; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_text;
	SwfTextAlignType m_textAlign;
};
	
	}
}

#endif	// traktor_flash_EditInstanceDebugInfo_H

