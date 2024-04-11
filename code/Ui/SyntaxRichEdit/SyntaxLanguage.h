/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Ui/SyntaxRichEdit/SyntaxTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Syntax highlight language.
 * \ingroup UI
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
		StSelf,
		StLineComment,
		StBlockComment,
		StFunction,
		StType,
		StKeyword,
		StSpecial,
		StPreprocessor
	};

	class IContext : public IRefCount {};

	/*! Return line comment token.
	 */
	virtual std::wstring lineComment() const = 0;

	/*! Create consume context. */
	virtual Ref< SyntaxLanguage::IContext > createContext() const = 0;

	/*! Consume line text.
	 *
	 * \param context Consume context.
	 * \param text Single line of text, same line will continue until fully consumed.
	 * \param outState Output text state.
	 * \param outConsumedChars Number of consumed characters.
	 * \return True if successful.
	 */
	virtual bool consume(SyntaxLanguage::IContext* context, const std::wstring& text, State& outState, int& outConsumedChars) const = 0;

	/*! Extract code outline.
	 *
	 * \param line Line number.
	 * \param text Single line of text.
	 * \param outFunctions List of defined functions.
	 */
	virtual void outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const = 0;
};

}
