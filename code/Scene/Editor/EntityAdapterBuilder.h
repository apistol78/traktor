#ifndef traktor_scene_EntityAdapterBuilder_H
#define traktor_scene_EntityAdapterBuilder_H

#include <map>
#include "Core/Heap/Ref.h"
#include "World/Entity/IEntityBuilder.h"

namespace traktor
{
	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;

class EntityAdapterBuilder : public world::IEntityBuilder
{
	T_RTTI_CLASS(EntityAdapterBuilder)

public:
	EntityAdapterBuilder(SceneEditorContext* context);

	virtual void addFactory(world::IEntityFactory* entityFactory);

	virtual void removeFactory(world::IEntityFactory* entityFactory);

	virtual void begin(world::IEntityManager* entityManager);

	virtual world::Entity* create(const std::wstring& name, const world::EntityData* entityData);

	virtual world::Entity* build(const world::EntityInstance* instance);

	virtual void end();

	EntityAdapter* getRootAdapter() const;

private:
	Ref< SceneEditorContext > m_context;
	Ref< world::IEntityManager > m_entityManager;
	RefArray< world::IEntityFactory > m_entityFactories;
	std::map< const world::EntityInstance*, Ref< EntityAdapter > > m_cachedInstances;
	Ref< EntityAdapter > m_currentAdapter;
	Ref< EntityAdapter > m_rootAdapter;
};

	}
}

#endif	// traktor_scene_EntityAdapterBuilder_H
