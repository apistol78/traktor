/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor::sound
{

class IGrainData;

class BankControlGrain : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	BankControlGrain(BankControlGrain* parent, IGrainData* grain, const std::wstring& text, ui::StyleBitmap* image);

	BankControlGrain* getParent() const;

	IGrainData* getGrain() const;

	ui::StyleBitmap* getImage() const;

	std::wstring getText() const;

	void setActive(bool active);

	virtual void mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position) override final;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) override final;

private:
	Ref< ui::StyleBitmap > m_bitmapGrain[4];
	BankControlGrain* m_parent;
	Ref< IGrainData > m_grain;
	std::wstring m_text;
	Ref< ui::StyleBitmap > m_image;
	bool m_active;
};

}
