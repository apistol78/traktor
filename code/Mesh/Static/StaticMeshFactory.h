#ifndef traktor_mesh_StaticMeshFactory_H
#define traktor_mesh_StaticMeshFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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

class T_DLLCLASS StaticMeshFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(StaticMeshFactory)

public:
	StaticMeshFactory(db::Database* database, render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory = 0);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_database;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::MeshFactory > m_meshFactory;
};

	}
}

#endif	// traktor_mesh_StaticMeshFactory_H
