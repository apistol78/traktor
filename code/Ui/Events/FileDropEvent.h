/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Io/Path.h"
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

/*! File drag'n'drop event.
 * \ingroup UI
 */
class T_DLLCLASS FileDropEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit FileDropEvent(EventSubject* sender, const std::vector< Path >& files);

	const std::vector< Path >& getFiles() const;

private:
	std::vector< Path > m_files;
};

	}
}

