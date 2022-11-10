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
#include "Render/ICubeTexture.h"

namespace traktor
{
	namespace render
	{

class ResourceTracker;

/*!
 * \ingroup Vrfy
 */
class CubeTextureVrfy : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	explicit CubeTextureVrfy(ResourceTracker* resourceTracker, ICubeTexture* texture);

	virtual ~CubeTextureVrfy();

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getSide() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	ICubeTexture* getTexture() const { return m_texture; }

private:
	Ref< ResourceTracker > m_resourceTracker;
	Ref< ICubeTexture > m_texture;
	int32_t m_locked[2];
};

	}
}
