/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Mesh/IMesh.h"
#include "Mesh/IMeshResource.h"
#include "Mesh/MeshFactory.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Render/Mesh/RenderMeshFactory.h"

namespace traktor
{
	namespace mesh
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
	return makeTypeInfoSet< IMeshResource >();
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
	Ref< IMeshResource > resource = instance->getObject< IMeshResource >();
	if (!resource)
	{
		log::error << L"Mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< IStream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	Ref< IMesh > mesh = resource->createMesh(instance->getPath(), dataStream, resourceManager, m_renderSystem, m_meshFactory);
	if (!mesh)
	{
		log::error << L"Mesh factory failed; unable to create mesh" << Endl;
		return 0;
	}
	
	return mesh;
}

	}
}
