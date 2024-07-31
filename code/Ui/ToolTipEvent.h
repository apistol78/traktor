/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Ui/Event.h"
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Tool tip event.
 * \ingroup UI
 */
class T_DLLCLASS ToolTipEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit ToolTipEvent(EventSubject* sender, const Point& position);

	void setPosition(const Point& position);

	const Point& getPosition() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

private:
	Point m_position;
	std::wstring m_text;
};

}
