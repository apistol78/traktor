#ifndef traktor_mesh_BlendMeshFactory_H
#define traktor_mesh_BlendMeshFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

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

class RenderSystem;
class MeshFactory;

	}

	namespace mesh
	{

class T_DLLCLASS BlendMeshFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(BlendMeshFactory)

public:
	BlendMeshFactory(db::Database* db, render::RenderSystem* renderSystem, render::MeshFactory* meshFactory = 0);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
	Ref< render::RenderSystem > m_renderSystem;
	Ref< render::MeshFactory > m_meshFactory;
};

	}
}

#endif	// traktor_mesh_BlendMeshFactory_H
