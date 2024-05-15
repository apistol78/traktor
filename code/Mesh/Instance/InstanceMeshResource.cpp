/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::mesh
{
	namespace
	{

const resource::Id< render::Shader > c_shaderInstanceMeshDraw(L"{A8FDE33C-D75B-4D4E-848F-7D7CF97F11D0}");

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshResource", 8, InstanceMeshResource, MeshResource)

Ref< IMesh > InstanceMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< render::Mesh > renderMesh;

	if (m_haveRenderMesh)
	{
		renderMesh = render::MeshReader(meshFactory).read(dataStream);
		if (!renderMesh)
		{
			log::error << L"Instance mesh create failed; unable to read mesh." << Endl;
			return nullptr;
		}
	}

	resource::Proxy< render::Shader > shaderDraw;
	if (!resourceManager->bind(c_shaderInstanceMeshDraw, shaderDraw))
		return nullptr;

	Ref< InstanceMesh > instanceMesh = new InstanceMesh(renderSystem, shaderDraw);

	if (!resourceManager->bind(m_shader, instanceMesh->m_shader))
		return nullptr;

	instanceMesh->m_renderMesh = renderMesh;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		instanceMesh->m_parts[worldTechnique].reserve(i->second.size());
		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			InstanceMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			instanceMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	return instanceMesh;
}

void InstanceMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 8, L"Incorrect version");

	MeshResource::serialize(s);

	s >> Member< bool >(L"haveRenderMesh", m_haveRenderMesh);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlMap<
		std::wstring,
		parts_t,
		Member< std::wstring >,
		MemberStlList< Part, MemberComposite< Part > >
	>(L"parts", m_parts);
}

void InstanceMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}

}
