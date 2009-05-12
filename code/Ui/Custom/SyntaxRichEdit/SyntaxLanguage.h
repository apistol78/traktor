#ifndef traktor_ui_custom_SyntaxLanguage_H
#define traktor_ui_custom_SyntaxLanguage_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(SyntaxLanguage)

public:
	enum State
	{
		StInvalid,
		StDefault,
		StString,
		StNumber,
		StComment,
		StKeyword
	};

	virtual void begin() = 0;

	/*! \brief Consume line text.
	 *
	 * \param text Single line of text, same line will continue until fully consumed.
	 * \param outState Output text state.
	 * \param outConsumedChars Number of consumed characters.
	 * \return True if successful.
	 */
	virtual bool consume(const std::wstring& text, State& outState, int& outConsumedChars) = 0;

	virtual void newLine() = 0;
};

		}
	}
}

#endif	// traktor_ui_custom_SyntaxLanguage_H
