/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Glsl/GlslResource.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslResource", GlslResource, Object)
	
GlslResource::GlslResource(const std::wstring& name, int32_t set, uint8_t stages)
:	m_name(name)
,	m_set(set)
,	m_stages(stages)
{
}

}
