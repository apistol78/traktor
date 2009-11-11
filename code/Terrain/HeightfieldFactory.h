#ifndef traktor_terrain_HeightfieldFactory_H
#define traktor_terrain_HeightfieldFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

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

class IRenderSystem;

	}

	namespace terrain
	{

class T_DLLCLASS HeightfieldFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(HeightfieldFactory)

public:
	HeightfieldFactory(db::Database* database, render::IRenderSystem* renderSystem);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_database;
	Ref< render::IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_terrain_HeightfieldFactory_H
