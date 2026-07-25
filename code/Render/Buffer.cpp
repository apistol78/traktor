/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Buffer.h"

#include "Render/ResourceMorgue.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Buffer", Buffer, Object)

void Buffer::release(void* owner) const noexcept
{
	if (--m_refCount == 0)
		ResourceMorgue::getInstance().retire(this);
}

uint32_t Buffer::getBufferSize() const
{
	return m_bufferSize;
}

Buffer::Buffer(uint32_t bufferSize)
:	m_bufferSize(bufferSize)
{
}

}
