/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ResourceTracker.h"
#include "Render/Vrfy/VolumeTextureVrfy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureVrfy", VolumeTextureVrfy, IVolumeTexture)

VolumeTextureVrfy::VolumeTextureVrfy(ResourceTracker* resourceTracker, IVolumeTexture* texture)
:	m_resourceTracker(resourceTracker)
,	m_texture(texture)
{
	m_resourceTracker->add(this);
}

VolumeTextureVrfy::~VolumeTextureVrfy()
{
	m_resourceTracker->remove(this);
}

void VolumeTextureVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* VolumeTextureVrfy::resolve()
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return this;
}

int32_t VolumeTextureVrfy::getWidth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getWidth() : 0;
}

int32_t VolumeTextureVrfy::getHeight() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getHeight() : 0;
}

int32_t VolumeTextureVrfy::getDepth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getDepth() : 0;
}

int32_t VolumeTextureVrfy::getMips() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getMips() : 0;
}

	}
}
