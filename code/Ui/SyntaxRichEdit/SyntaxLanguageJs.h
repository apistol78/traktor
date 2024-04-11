/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/SyntaxRichEdit/SyntaxLanguage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Syntax highlight JavaScript language.
 * \ingroup UI
 */
class T_DLLCLASS SyntaxLanguageJs : public SyntaxLanguage
{
	T_RTTI_CLASS;

public:
	virtual std::wstring lineComment() const override final;

	virtual Ref< SyntaxLanguage::IContext > createContext() const override final;

	virtual bool consume(SyntaxLanguage::IContext* context, const std::wstring& text, State& outState, int& outConsumedChars) const override final;

	virtual void outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const override final;
};

}
