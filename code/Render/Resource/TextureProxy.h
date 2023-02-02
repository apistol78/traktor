/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ITexture.h"
#include "Resource/Proxy.h"

namespace traktor::render
{

class TextureProxy : public ITexture
{
	T_RTTI_CLASS;

public:
	explicit TextureProxy(const resource::Proxy< ITexture >& texture);

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual ITexture* resolve() override final;

private:
	resource::Proxy< ITexture > m_texture;
};

}
