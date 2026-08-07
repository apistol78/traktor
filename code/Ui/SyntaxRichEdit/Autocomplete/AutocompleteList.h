/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class AutocompleteItem;

/*! Autocomplete suggestion list widget.
 * \ingroup UI
 */
class T_DLLCLASS AutocompleteList : public AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent);

	void addItem(AutocompleteItem* item);

	void removeAll();

	int32_t count() const;

	AutocompleteItem* getItem(int32_t index) const;

	AutocompleteItem* getSelectedItem() const;

	int32_t getSelectedIndex() const;

	void setSelectedIndex(int32_t index);

	bool selectNext();

	bool selectPrevious();

private:
	RefArray< AutocompleteItem > m_items;

	virtual void layoutCells(const Rect& rc) override final;

	void eventButtonDown(MouseButtonDownEvent* event);
};

}
