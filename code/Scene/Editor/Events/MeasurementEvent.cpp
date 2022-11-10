/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/Events/MeasurementEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.MeasurementEvent", MeasurementEvent, ui::Event)

MeasurementEvent::MeasurementEvent(ui::EventSubject* sender, int32_t pass, int32_t level, const std::wstring& name, double start, double duration)
:	ui::Event(sender)
,   m_pass(pass)
,	m_level(level)
,   m_name(name)
,   m_start(start)
,   m_duration(duration)
{
}

	}
}
