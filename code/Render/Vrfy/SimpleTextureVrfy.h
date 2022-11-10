/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ResourceTracker;

/*!
 * \ingroup Vrfy
 */
class SimpleTextureVrfy : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	explicit SimpleTextureVrfy(ResourceTracker* resourceTracker, ISimpleTexture* texture);

	virtual ~SimpleTextureVrfy();

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	ISimpleTexture* getTexture() const { return m_texture; }

private:
	Ref< ResourceTracker > m_resourceTracker;
	Ref< ISimpleTexture > m_texture;
	int32_t m_locked;
};

	}
}
