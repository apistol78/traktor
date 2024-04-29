/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Edge;

/*! Node or edge selection event.
 * \ingroup UI
 */
class T_DLLCLASS EdgeConnectEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit EdgeConnectEvent(EventSubject* sender, Edge* edge);

	Edge* getEdge() const;

private:
	Ref< Edge > m_edge;
};

}
