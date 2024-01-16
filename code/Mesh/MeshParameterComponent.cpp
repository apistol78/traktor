/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/MeshComponent.h"
#include "Mesh/MeshParameterComponent.h"
#include "Render/ITexture.h"
#include "Render/Context/ProgramParameters.h"
#include "World/Entity.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshParameterComponent", MeshParameterComponent, world::IEntityComponent)

MeshParameterComponent::MeshParameterComponent(const SmallMap< render::handle_t, resource::Proxy< render::ITexture > >& textures)
:	m_textures(textures)
{
}

void MeshParameterComponent::destroy()
{
}

void MeshParameterComponent::setOwner(world::Entity* owner)
{
	// Remove ourself from previous owner's mesh component.
	if (m_owner != nullptr)
	{
		MeshComponent* mesh = m_owner->getComponent< MeshComponent >();
		if (mesh)
			mesh->setParameterCallback(nullptr);
	}

	// Add ourself to owner's mesh component.
	if ((m_owner = owner) != nullptr)
	{
		MeshComponent* mesh = owner->getComponent< MeshComponent >();
		if (mesh)
			mesh->setParameterCallback(this);
	}
}

void MeshParameterComponent::setTransform(const Transform& transform)
{
}

Aabb3 MeshParameterComponent::getBoundingBox() const
{
	return Aabb3();
}

void MeshParameterComponent::update(const world::UpdateParams& update)
{
}

void MeshParameterComponent::setVectorParameter(const render::handle_t parameter, const Vector4& value)
{
	m_values[parameter] = value;
}

void MeshParameterComponent::setTextureParameter(const render::handle_t parameter, render::ITexture* texture)
{
	m_textures[parameter] = resource::Proxy< render::ITexture >(texture);
}

void MeshParameterComponent::setParameters(render::ProgramParameters* programParameters) const
{
	for (auto it : m_values)
		programParameters->setVectorParameter(it.first, it.second);
	for (auto it : m_textures)
		programParameters->setTextureParameter(it.first, it.second);
}

}
