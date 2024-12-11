/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ResourceTracker.h"
#include "Render/Vrfy/TextureVrfy.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureVrfy", TextureVrfy, ITexture)

TextureVrfy::TextureVrfy(ResourceTracker* resourceTracker, ITexture* texture, bool shaderStorage)
:	m_resourceTracker(resourceTracker)
,	m_texture(texture)
,	m_shaderStorage(shaderStorage)
,	m_locked(-1)
{
	m_resourceTracker->add(this);
}

TextureVrfy::~TextureVrfy()
{
	m_resourceTracker->remove(this);
}

void TextureVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_texture, L"Texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture::Size TextureVrfy::getSize() const
{
	T_CAPTURE_ASSERT (m_texture, L"Texture destroyed.");
	return m_texture ? m_texture->getSize() : Size{ 0 };
}

int32_t TextureVrfy::getBindlessIndex() const
{
	T_CAPTURE_ASSERT (m_texture, L"Texture destroyed.");
	return m_texture ? m_texture->getBindlessIndex() : -1;
}

bool TextureVrfy::lock(int32_t side, int32_t level, Lock& lock)
{
	T_CAPTURE_ASSERT (m_texture, L"Texture destroyed.");
	T_CAPTURE_ASSERT (level >= 0, L"Invalid mip level.");
	T_CAPTURE_ASSERT (level < getSize().mips, L"Invalid mip level.");
	T_CAPTURE_ASSERT (m_locked < 0, L"Already locked.");
	return m_texture ? m_texture->lock(side, level, lock) : false;
}

void TextureVrfy::unlock(int32_t side, int32_t level)
{
	T_CAPTURE_ASSERT (m_texture, L"Texture destroyed.");
	T_CAPTURE_ASSERT (level >= 0, L"Invalid mip level.");
	T_CAPTURE_ASSERT (level < getSize().mips, L"Invalid mip level.");
	T_CAPTURE_ASSERT (m_locked != level, L"Trying to unlock incorrect mip level.");
	if (m_texture)
		m_texture->unlock(side, level);
	m_locked = -1;
}

ITexture* TextureVrfy::resolve()
{
	T_CAPTURE_ASSERT (m_texture, L"Texture destroyed.");
	return this;
}

}
