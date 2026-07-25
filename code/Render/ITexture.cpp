/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/ITexture.h"

#include "Render/ResourceMorgue.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ITexture", ITexture, Object)

void ITexture::release(void* owner) const noexcept
{
	if (--m_refCount == 0)
		ResourceMorgue::getInstance().retire(this);
}

}
