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
	
class EditInstance;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS EditInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	EditInstanceDebugInfo();

	EditInstanceDebugInfo(const EditInstance* instance);

	const std::wstring& getText() const { return m_text; }

	const Color4f& getTextColor() const { return m_textColor; }

	SwfTextAlignType getTextAlign() const { return m_textAlign; }

	const Vector2& getTextSize() const { return m_textSize; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_text;
	Color4f m_textColor;
	SwfTextAlignType m_textAlign;
	Vector2 m_textSize;
};
	
	}
}

#endif	// traktor_flash_EditInstanceDebugInfo_H

