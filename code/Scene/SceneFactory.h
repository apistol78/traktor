#ifndef traktor_scene_SceneFactory_H
#define traktor_scene_SceneFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
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

	namespace world
	{

class IEntityBuilder;

	}

	namespace scene
	{

class T_DLLCLASS SceneFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(SceneFactory)

public:
	SceneFactory(db::Database* database, world::IEntityBuilder* entityBuilder);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Object* create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_database;
	Ref< world::IEntityBuilder > m_entityBuilder;
};

	}
}

#endif	// traktor_scene_SceneFactory_H
