#ifndef traktor_mesh_SkinnedMeshFactory_H
#define traktor_mesh_SkinnedMeshFactory_H

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

	}

	namespace mesh
	{

class T_DLLCLASS SkinnedMeshFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(SkinnedMeshFactory)

public:
	SkinnedMeshFactory(db::Database* database, render::RenderSystem* renderSystem);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_database;
	Ref< render::RenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshFactory_H
