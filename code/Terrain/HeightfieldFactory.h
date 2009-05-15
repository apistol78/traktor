#ifndef traktor_terrain_HeightfieldFactory_H
#define traktor_terrain_HeightfieldFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
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

	namespace terrain
	{

class T_DLLCLASS HeightfieldFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(HeightfieldFactory)

public:
	HeightfieldFactory(db::Database* database, render::RenderSystem* renderSystem);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid, bool& outCacheable);

private:
	Ref< db::Database > m_database;
	Ref< render::RenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_terrain_HeightfieldFactory_H
