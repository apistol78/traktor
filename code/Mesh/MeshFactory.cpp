/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Mesh/IMesh.h"
#include "Mesh/MeshResource.h"
#include "Mesh/MeshFactory.h"
#include "Render/Mesh/RenderMeshFactory.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshFactory", MeshFactory, resource::IResourceFactory)

MeshFactory::MeshFactory(render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = new render::RenderMeshFactory(m_renderSystem);
}

const TypeInfoSet MeshFactory::getResourceTypes() const
{
	return makeTypeInfoSet< MeshResource >();
}

const TypeInfoSet MeshFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< IMesh >();
}

bool MeshFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > MeshFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< MeshResource > resource = instance->getObject< MeshResource >();
	if (!resource)
	{
		log::error << L"Mesh factory failed; unable to get mesh resource." << Endl;
		return nullptr;
	}

	Ref< IStream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Mesh factory failed; unable to open data stream." << Endl;
		return nullptr;
	}

	if (resource->isCompressed())
		dataStream = new compress::InflateStreamLzf(dataStream);

	Ref< IMesh > mesh = resource->createMesh(instance->getPath(), dataStream, resourceManager, m_renderSystem, m_meshFactory);
	if (!mesh)
	{
		log::error << L"Mesh factory failed; unable to create mesh." << Endl;
		return nullptr;
	}

	if (!is_type_of(productType, type_of(mesh)))
	{
		log::error << L"Mesh factory failed; incorrect product type." << Endl;
		return nullptr;
	}

	return mesh;
}

}
