/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/ITexture.h"

namespace traktor::render
{

class ResourceTracker;

/*!
 * \ingroup Vrfy
 */
class TextureVrfy : public ITexture
{
	T_RTTI_CLASS;

public:
	explicit TextureVrfy(ResourceTracker* resourceTracker, ITexture* texture, bool shaderStorage);

	virtual ~TextureVrfy();

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual ITexture* resolve() override final;

	ITexture* getTexture() const { return m_texture; }

	bool shaderStorage() const { return m_shaderStorage; }

private:
	Ref< ResourceTracker > m_resourceTracker;
	Ref< ITexture > m_texture;
	bool m_shaderStorage;
	int32_t m_locked;
};

}
