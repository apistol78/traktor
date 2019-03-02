#pragma once

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

/*! \brief Syntax highlight JavaScript language.
 * \ingroup UI
 */
class T_DLLCLASS SyntaxLanguageJs : public SyntaxLanguage
{
	T_RTTI_CLASS;

public:
	virtual std::wstring lineComment() const override final;

	virtual bool consume(const std::wstring& text, State& outState, int& outConsumedChars) const override final;

	virtual void outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const override final;
};

	}
}

