/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/Events/ModifierChangedEvent.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ModifierChangedEvent", ModifierChangedEvent, ui::Event)

ModifierChangedEvent::ModifierChangedEvent(ui::EventSubject* sender)
:	ui::Event(sender)
{
}

}
