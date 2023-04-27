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
#include "Render/ITexture.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class ApiBuffer;
class Context;
class Image;

struct SimpleTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class TextureVk : public ITexture
{
	T_RTTI_CLASS;

public:
	explicit TextureVk(Context* context, uint32_t& instances);

	virtual ~TextureVk();

	bool create(
		const SimpleTextureCreateDesc& desc,
		const wchar_t* const tag
	);

	bool create(
		const CubeTextureCreateDesc& desc,
		const wchar_t* const tag
	);

	bool create(
		const VolumeTextureCreateDesc& desc,
		const wchar_t* const tag
	);

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual ITexture* resolve() override final;

	Image& getImage() const { return *m_textureImage; }

	uint32_t getBindlessResourceIndex() const { return m_bindlessResourceIndex; }

private:
	Context* m_context = nullptr;
	uint32_t& m_instances;
	Ref< ApiBuffer > m_stagingBuffer;
	Ref< Image > m_textureImage;
	ITexture::Size m_size;
	TextureFormat m_format;

	uint32_t m_bindlessResourceIndex;
};

}
