/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Static/StaticMeshResource.h"

#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldTypes.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshResource", 7, StaticMeshResource, MeshResource)

StaticMeshResource::StaticMeshResource()
	: m_haveRenderMesh(false)
{
}

Ref< IMesh > StaticMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory) const
{
	Ref< StaticMesh > staticMesh = new StaticMesh();

	if (!resourceManager->bind(m_shader, staticMesh->m_shader))
		return nullptr;

	staticMesh->m_albedoTextures.resize(m_albedoTextures.size());
	for (size_t i = 0; i < m_albedoTextures.size(); ++i)
		if (!resourceManager->bind(m_albedoTextures[i], staticMesh->m_albedoTextures[i]))
			return nullptr;

	// Create rasterization parts.
	for (const auto& tp : m_parts)
	{
		const render::handle_t worldTechnique = render::getParameterHandle(tp.first);

		auto& r = staticMesh->m_techniqueParts[worldTechnique];
		r.first = (uint32_t)staticMesh->m_parts.size();

		staticMesh->m_parts.reserve(r.first + tp.second.size());
		for (const StaticMeshResource::Part& resourcePart : tp.second)
		{
			StaticMesh::Part& part = staticMesh->m_parts.push_back();
			part.shaderTechnique = render::getParameterHandle(resourcePart.shaderTechnique);
			part.meshPart = resourcePart.meshPart;
		}

		r.second = (uint32_t)staticMesh->m_parts.size();
	}

	Ref< render::Mesh > renderMesh;
	if (m_haveRenderMesh)
	{
		auto fn = [&](void* ptr) {
			world::HWRT_Material* attr = (world::HWRT_Material*)ptr;
			if (attr->albedoMap >= 0 && attr->albedoMap < staticMesh->m_albedoTextures.size() && staticMesh->m_albedoTextures[attr->albedoMap] != nullptr)
				attr->albedoMap = staticMesh->m_albedoTextures[attr->albedoMap]->getBindlessIndex();
			else
				attr->albedoMap = -1;
		};

		renderMesh = render::MeshReader(meshFactory).read(dataStream, { { .id = IMesh::c_fccRayTracingVertexAttributes, .elementSize = sizeof(world::HWRT_Material), .fn = fn } });
		if (!renderMesh)
		{
			log::error << L"Static mesh create failed; unable to read render mesh." << Endl;
			return nullptr;
		}
	}

	staticMesh->m_renderMesh = renderMesh;

	// Create ray tracing structures.
	if (renderSystem->supportRayTracing())
	{
		AlignedVector< render::Primitives > primitives;
		for (const auto& part : renderMesh->getParts())
		{
			if (part.raytracing)
				primitives.push_back(part.primitives);
		}

		staticMesh->m_rtAccelerationStructure = renderSystem->createAccelerationStructure(
			renderMesh->getVertexBuffer(),
			renderMesh->getVertexLayout(),
			renderMesh->getIndexBuffer(),
			renderMesh->getIndexType(),
			primitives,
			false);
		if (!staticMesh->m_rtAccelerationStructure)
		{
			log::error << L"Static mesh create failed; unable to create RT acceleration structure." << Endl;
			return nullptr;
		}
	}

#if defined(_DEBUG)
	staticMesh->m_name = wstombs(name);
#endif
	return staticMesh;
}

void StaticMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 7, L"Incorrect version");

	MeshResource::serialize(s);

	s >> Member< bool >(L"haveRenderMesh", m_haveRenderMesh);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< resource::Id< render::ITexture >, resource::Member< render::ITexture > >(L"albedoTextures", m_albedoTextures);
	s >> MemberSmallMap<
			 std::wstring,
			 parts_t,
			 Member< std::wstring >,
			 MemberAlignedVector< Part, MemberComposite< Part > > >(L"parts", m_parts);
}

void StaticMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}
}
