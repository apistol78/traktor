/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Ui/Auto/AutoWidgetCell.h"
#include "Ui/SyntaxRichEdit/Autocomplete/IAutocompleteProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;

/*! Autocomplete suggestion item cell.
 * \ingroup UI
 */
class T_DLLCLASS AutocompleteItem : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	AutocompleteItem(const AutocompleteSuggestion& suggestion, IBitmap* icon);

	const AutocompleteSuggestion& getSuggestion() const { return m_suggestion; }

	void setSelected(bool selected);

	bool isSelected() const { return m_selected; }

	virtual void paint(Canvas& canvas, const Rect& rect) override final;

private:
	AutocompleteSuggestion m_suggestion;
	Ref< IBitmap > m_icon;
	bool m_selected = false;
};

}
