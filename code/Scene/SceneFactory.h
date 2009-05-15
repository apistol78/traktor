#ifndef traktor_scene_SceneFactory_H
#define traktor_scene_SceneFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

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

class EntityBuilder;

	}

	namespace scene
	{

class T_DLLCLASS SceneFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(SceneFactory)

public:
	SceneFactory(db::Database* database, world::EntityBuilder* entityBuilder);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid, bool& outCacheable);

private:
	Ref< db::Database > m_database;
	Ref< world::EntityBuilder > m_entityBuilder;
};

	}
}

#endif	// traktor_scene_SceneFactory_H
