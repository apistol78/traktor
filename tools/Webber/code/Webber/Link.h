/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Webber/Event.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

/*! Clickable text link.
 * \ingroup Webber
 */
class Link : public Widget
{
	T_RTTI_CLASS;

public:
	explicit Link(const std::wstring& text);

	virtual void build(html::Element* parent) const override final;

	virtual void consume(int32_t senderId, int32_t action) override final;

	Event& clicked();

private:
	std::wstring m_text;
	Event m_clicked;
};

}
