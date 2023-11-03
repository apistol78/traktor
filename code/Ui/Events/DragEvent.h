/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Tree view drag event.
 * \ingroup UI
 */
class T_DLLCLASS DragEvent : public Event
{
	T_RTTI_CLASS;

public:
	enum class Moment
	{
		Drag,
		Drop
	};

	explicit DragEvent(EventSubject* sender, Moment moment, const Point& position = Point(0, 0));

	Moment getMoment() const;

	const Point& getPosition() const;

	void cancel();

	bool cancelled() const;

private:
	Moment m_moment;
	Point m_position;
	bool m_cancelled;
};

}
