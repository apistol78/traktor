/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshResource", 6, StaticMeshResource, MeshResource)

StaticMeshResource::StaticMeshResource()
:	m_haveRenderMesh(false)
{
}

Ref< IMesh > StaticMeshResource::createMesh(
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
			log::error << L"Static mesh create failed; unable to read render mesh." << Endl;
			return nullptr;
		}
	}

	Ref< StaticMesh > staticMesh = new StaticMesh();

	if (!resourceManager->bind(m_shader, staticMesh->m_shader))
		return nullptr;

	staticMesh->m_renderMesh = renderMesh;

	for (const auto& tp : m_parts)
	{
		const render::handle_t worldTechnique = render::getParameterHandle(tp.first);

		auto& r = staticMesh->m_techniqueParts[worldTechnique];
		r.first = (uint32_t)staticMesh->m_parts.size();

		staticMesh->m_parts.reserve(r.first + tp.second.size());
		for (const auto& p : tp.second)
		{
			StaticMesh::Part& part = staticMesh->m_parts.push_back();
			part.shaderTechnique = render::getParameterHandle(p.shaderTechnique);
			part.meshPart = p.meshPart;
		}

		r.second = (uint32_t)staticMesh->m_parts.size();
	}

#if defined(_DEBUG)
	staticMesh->m_name = wstombs(name);
#endif
	return staticMesh;
}

void StaticMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 6, L"Incorrect version");

	MeshResource::serialize(s);

	s >> Member< bool >(L"haveRenderMesh", m_haveRenderMesh);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberSmallMap<
		std::wstring,
		parts_t,
		Member< std::wstring >,
		MemberAlignedVector< Part, MemberComposite< Part > >
	>(L"parts", m_parts);
}

void StaticMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}

}
