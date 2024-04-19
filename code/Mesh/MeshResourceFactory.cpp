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
#include "Core/Misc/ObjectStore.h"
#include "Database/Instance.h"
#include "Mesh/IMesh.h"
#include "Mesh/MeshResource.h"
#include "Mesh/MeshResourceFactory.h"
#include "Render/IRenderSystem.h"
#include "Render/Mesh/RenderMeshFactory.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshResourceFactory", 0, MeshResourceFactory, resource::IResourceFactory)

MeshResourceFactory::MeshResourceFactory(render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = new render::RenderMeshFactory(m_renderSystem);
}

bool MeshResourceFactory::initialize(const ObjectStore& objectStore)
{
	m_renderSystem = objectStore.get< render::IRenderSystem >();
	m_meshFactory = objectStore.get< render::MeshFactory >();

	if (!m_meshFactory)
		m_meshFactory = new render::RenderMeshFactory(m_renderSystem);

	return true;
}

const TypeInfoSet MeshResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< MeshResource >();
}

const TypeInfoSet MeshResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< IMesh >();
}

bool MeshResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > MeshResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const MeshResource > resource = instance->getObject< MeshResource >();
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
