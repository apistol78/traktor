/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/Buffer.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class BufferDx11 : public Buffer
{
	T_RTTI_CLASS;

protected:
	Ref< ContextDx11 > m_context;

	explicit BufferDx11(ContextDx11* context, uint32_t elementCount, uint32_t elementSize);
};

	}
}
