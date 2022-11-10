/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Render/ITexture.h"
#include "Render/Resource/AliasTextureResource.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.AliasTextureResource", 0, AliasTextureResource, ISerializable)

AliasTextureResource::AliasTextureResource(const resource::Id< ITexture >& texture)
:	m_texture(texture)
{
}

void AliasTextureResource::serialize(ISerializer& s)
{
	s >> resource::Member< ITexture >(L"texture", m_texture);
}

	}
}
