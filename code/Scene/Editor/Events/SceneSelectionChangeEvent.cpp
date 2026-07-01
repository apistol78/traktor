/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/Events/SceneSelectionChangeEvent.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneSelectionChangeEvent", SceneSelectionChangeEvent, ui::SelectionChangeEvent)

SceneSelectionChangeEvent::SceneSelectionChangeEvent(ui::EventSubject* sender, bool ensureEntityVisible)
	: ui::SelectionChangeEvent(sender)
	, m_ensureEntityVisible(ensureEntityVisible)
{
}

}
