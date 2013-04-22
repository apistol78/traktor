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
	namespace db
	{

class Database;

	}

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
	MeshFactory(db::Database* database, render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory = 0);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const;

private:
	Ref< db::Database > m_database;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::MeshFactory > m_meshFactory;
};

	}
}

#endif	// traktor_mesh_MeshFactory_H
