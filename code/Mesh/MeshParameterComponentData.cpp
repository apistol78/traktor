/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Mesh/MeshParameterComponent.h"
#include "Mesh/MeshParameterComponentData.h"
#include "Render/ITexture.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshParameterComponentData", 1, MeshParameterComponentData, world::IEntityComponentData)

Ref< MeshParameterComponent > MeshParameterComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	SmallMap< render::handle_t, resource::Proxy< render::ITexture > > textures;
	for (auto it : m_textures)
	{
		resource::Proxy< render::ITexture > texture;
		if (!resourceManager->bind(it.second, texture))
			return nullptr;

		textures.insert(
			render::getParameterHandle(it.first),
			texture
		);
	}

	return new MeshParameterComponent(textures);
}

void MeshParameterComponentData::setTexture(const std::wstring& parameterName, const resource::Id< render::ITexture >& texture)
{
	m_textures[parameterName] = texture;
}

int32_t MeshParameterComponentData::getOrdinal() const
{
	return -10;
}

void MeshParameterComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void MeshParameterComponentData::serialize(ISerializer& s)
{
	s >> MemberSmallMap<
		std::wstring,
		resource::Id< render::ITexture >,
		Member< std::wstring >,
		resource::Member< render::ITexture >
	>(L"textures", m_textures);
}

}
