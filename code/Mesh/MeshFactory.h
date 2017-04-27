/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_MeshFactory_H
#define traktor_mesh_MeshFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class MeshFactory;

	}

	namespace mesh
	{

class T_DLLCLASS MeshFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	MeshFactory(render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory = 0);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::MeshFactory > m_meshFactory;
};

	}
}

#endif	// traktor_mesh_MeshFactory_H
