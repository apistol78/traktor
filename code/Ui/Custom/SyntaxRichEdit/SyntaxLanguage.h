/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_SyntaxLanguage_H
#define traktor_ui_custom_SyntaxLanguage_H

#include <list>
#include "Core/Object.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxTypes.h"

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

/*! \brief Syntax highlight language.
 * \ingroup UIC
 */
class T_DLLCLASS SyntaxLanguage : public Object
{
	T_RTTI_CLASS;

public:
	enum State
	{
		StInvalid,
		StDefault,
		StString,
		StNumber,
		StComment,
		StFunction,
		StType,
		StKeyword,
		StSpecial,
		StPreprocessor
	};

	/*! \brief Return line comment token.
	 */
	virtual std::wstring lineComment() const = 0;

	/*! \brief Consume line text.
	 *
	 * \param text Single line of text, same line will continue until fully consumed.
	 * \param outState Output text state.
	 * \param outConsumedChars Number of consumed characters.
	 * \return True if successful.
	 */
	virtual bool consume(const std::wstring& text, State& outState, int& outConsumedChars) const = 0;

	/*! \brief Extract code outline.
	 *
	 * \param line Line number.
	 * \param text Single line of text.
	 * \param outFunctions List of defined functions.
	 */
	virtual void outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const = 0;
};

		}
	}
}

#endif	// traktor_ui_custom_SyntaxLanguage_H
