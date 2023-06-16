/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Auto/AutoWidgetCell.h"

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

class Font;
class GridRow;
class IBitmap;

/*! Grid item.
 * \ingroup UI
 */
class T_DLLCLASS GridItem : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	GridItem() = default;

	explicit GridItem(const std::wstring& text);

	explicit GridItem(const std::wstring& text, Font* font);

	explicit GridItem(const std::wstring& text, IBitmap* image);

	explicit GridItem(IBitmap* image);

	void setText(const std::wstring& text);

	std::wstring getText() const;

	void setTextColor(const Color4ub& textColor);

	const Color4ub& getTextColor() const;

	bool edit();

	void setFont(Font* font);

	Font* getFont() const;

	void setImage(IBitmap* image);

	int32_t addImage(IBitmap* image);

	const RefArray< IBitmap >& getImages() const;

	int32_t getHeight();

	GridRow* getRow() const;

private:
	friend class GridRow;

	GridRow* m_row = nullptr;
	std::wstring m_text;
	Color4ub m_textColor = Color4ub(0, 0, 0, 0);
	Ref< Font > m_font;
	RefArray< IBitmap > m_images;

	void setOwner(AutoWidget* owner);

	virtual AutoWidgetCell* hitTest(const Point& position) override final;

	virtual void paint(Canvas& canvas, const Rect& rect) override final;
};

	}
}

