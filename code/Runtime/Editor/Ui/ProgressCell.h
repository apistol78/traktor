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

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class ProgressCell : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	ProgressCell();

	void setText(const std::wstring& text);

	void setProgress(int32_t progress);

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) override final;

private:
	std::wstring m_text;
	int32_t m_progress;
};

	}
}

