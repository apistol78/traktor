/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_SyntaxLanguageLua_H
#define traktor_ui_SyntaxLanguageLua_H

#include "Ui/SyntaxRichEdit/SyntaxLanguage.h"

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

/*! \brief Syntax highlight LUA language.
 * \ingroup UI
 */
class T_DLLCLASS SyntaxLanguageLua : public SyntaxLanguage
{
	T_RTTI_CLASS;

public:
	virtual std::wstring lineComment() const override final;

	virtual bool consume(const std::wstring& text, State& outState, int& outConsumedChars) const override final;

	virtual void outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const override final;
};

	}
}

#endif	// traktor_ui_SyntaxLanguageLua_H
