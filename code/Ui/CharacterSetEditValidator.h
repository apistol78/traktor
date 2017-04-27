/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CharacterSetEditValidator_H
#define traktor_ui_CharacterSetEditValidator_H

#include <set>
#include "Ui/EditValidator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Character set text edit validator.
 * \ingroup UI
 */
class T_DLLCLASS CharacterSetEditValidator : public EditValidator
{
	T_RTTI_CLASS;

public:
	void add(wchar_t ch);

	void addRange(wchar_t from, wchar_t to);

	virtual EditValidator::Result validate(const std::wstring& text) const T_OVERRIDE;

private:
	std::set< wchar_t > m_set;
};

	}
}

#endif	// traktor_ui_CharacterSetEditValidator_H
