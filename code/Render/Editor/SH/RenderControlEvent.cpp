/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/SH/RenderControlEvent.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderControlEvent", RenderControlEvent, ui::Event)

RenderControlEvent::RenderControlEvent(
	ui::EventSubject* sender,
	IRenderView* renderView,
	PrimitiveRenderer* primitiveRenderer
)
:	ui::Event(sender)
,	m_renderView(renderView)
,	m_primitiveRenderer(primitiveRenderer)
{
}

}
