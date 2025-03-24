/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Instance/InstanceMeshResource.h"

#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldTypes.h"

#include <algorithm>

namespace traktor::mesh
{
namespace
{

const resource::Id< render::Shader > c_shaderInstanceMeshDraw(L"{A8FDE33C-D75B-4D4E-848F-7D7CF97F11D0}");

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshResource", 9, InstanceMeshResource, MeshResource)

Ref< IMesh > InstanceMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory) const
{
	Ref< render::Mesh > renderMesh;

	resource::Proxy< render::Shader > shaderDraw;
	if (!resourceManager->bind(c_shaderInstanceMeshDraw, shaderDraw))
		return nullptr;

	Ref< InstanceMesh > instanceMesh = new InstanceMesh(renderSystem, shaderDraw);

	instanceMesh->m_albedoTextures.resize(m_albedoTextures.size());
	for (size_t i = 0; i < m_albedoTextures.size(); ++i)
		if (!resourceManager->bind(m_albedoTextures[i], instanceMesh->m_albedoTextures[i]))
			return nullptr;

	if (!resourceManager->bind(m_shader, instanceMesh->m_shader))
		return nullptr;

	if (m_haveRenderMesh)
	{
		auto fn = [&](void* ptr) {
			world::HWRT_Material* attr = (world::HWRT_Material*)ptr;
			if (attr->albedoMap >= 0 && attr->albedoMap < instanceMesh->m_albedoTextures.size() && instanceMesh->m_albedoTextures[attr->albedoMap] != nullptr)
				attr->albedoMap = instanceMesh->m_albedoTextures[attr->albedoMap]->getBindlessIndex();
			else
				attr->albedoMap = -1;
		};

		renderMesh = render::MeshReader(meshFactory).read(dataStream, { { .id = IMesh::c_fccRayTracingVertexAttributes, .elementSize = sizeof(world::HWRT_Material), .fn = fn } });
		if (!renderMesh)
		{
			log::error << L"Instance mesh create failed; unable to read mesh." << Endl;
			return nullptr;
		}
	}

	instanceMesh->m_renderMesh = renderMesh;

	for (const auto& part : m_parts)
	{
		render::handle_t worldTechnique = render::getParameterHandle(part.first);

		instanceMesh->m_parts[worldTechnique].reserve(part.second.size());
		for (parts_t::const_iterator j = part.second.begin(); j != part.second.end(); ++j)
		{
			InstanceMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			instanceMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	if (renderSystem->supportRayTracing())
	{
		const auto& part = renderMesh->getParts().back();
		T_FATAL_ASSERT(part.name == L"__RT__");

		AlignedVector< render::Primitives > primitives;
		primitives.push_back(part.primitives);

		instanceMesh->m_rtAccelerationStructure = renderSystem->createAccelerationStructure(
			renderMesh->getVertexBuffer(),
			renderMesh->getVertexLayout(),
			renderMesh->getIndexBuffer(),
			renderMesh->getIndexType(),
			primitives);
		if (!instanceMesh->m_rtAccelerationStructure)
		{
			log::error << L"Instance mesh create failed; unable to create RT acceleration structure." << Endl;
			return nullptr;
		}
	}

	return instanceMesh;
}

void InstanceMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 9, L"Incorrect version");

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

void InstanceMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
}

}
