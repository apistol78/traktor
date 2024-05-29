/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Event.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Canvas;

/*! Paint event.
 * \ingroup UI
 */
class T_DLLCLASS OverlayPaintEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit OverlayPaintEvent(EventSubject* sender, Canvas& canvas, const Rect& rc);

	Canvas& getCanvas() const;

	const Rect& getUpdateRect() const;

private:
	Canvas& m_canvas;
	Rect m_rc;
};

}
