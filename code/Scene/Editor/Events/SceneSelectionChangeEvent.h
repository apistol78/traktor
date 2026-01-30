/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Events/SelectionChangeEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class T_DLLCLASS SceneSelectionChangeEvent : public ui::SelectionChangeEvent
{
	T_RTTI_CLASS;

public:
	explicit SceneSelectionChangeEvent(ui::EventSubject* sender, bool ensureEntityVisible);

	bool shouldEnsureEntityVisible() const { return m_ensureEntityVisible; }

private:
	bool m_ensureEntityVisible = false;
};

}
