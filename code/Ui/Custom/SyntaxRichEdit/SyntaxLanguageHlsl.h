#ifndef traktor_ui_custom_SyntaxLanguageHlsl_H
#define traktor_ui_custom_SyntaxLanguageHlsl_H

#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Syntax highlight HLSL or CG language.
 * \ingroup UIC
 */
class T_DLLCLASS SyntaxLanguageHlsl : public SyntaxLanguage
{
	T_RTTI_CLASS;

public:
	virtual std::wstring lineComment() const T_OVERRIDE T_FINAL;

	virtual bool consume(const std::wstring& text, State& outState, int& outConsumedChars) const T_OVERRIDE T_FINAL;

	virtual void outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const T_OVERRIDE T_FINAL;
};

		}
	}
}

#endif	// traktor_ui_custom_SyntaxLanguageHlsl_H
