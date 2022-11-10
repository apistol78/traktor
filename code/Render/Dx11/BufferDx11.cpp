/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Dx11/BufferDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferDx11", BufferDx11, Buffer)

BufferDx11::BufferDx11(ContextDx11* context, uint32_t elementCount, uint32_t elementSize)
:	Buffer(elementCount, elementSize)
,	m_context(context)
{
}

	}
}