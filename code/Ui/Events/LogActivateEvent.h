/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Guid.h"
#include "Ui/Event.h"

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

/*! Log symbol activated.
 * \ingroup UI
 *
 * Carry all symbols found in the activated log line,
 * in order of appearance.
 */
class T_DLLCLASS LogActivateEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit LogActivateEvent(EventSubject* sender, const AlignedVector< Guid >& symbolIds);

	const Guid& getSymbolId() const;

	const AlignedVector< Guid >& getSymbolIds() const;

private:
	AlignedVector< Guid > m_symbolIds;
};

	}
}
