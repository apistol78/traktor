#ifndef traktor_animation_ClothEntityFactory_H
#define traktor_animation_ClothEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace animation
	{

/*! \brief Cloth entity factory.
 * \ingroup Animation
 */
class T_DLLCLASS ClothEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	ClothEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;

private:
	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
};

	}
}

#endif	// traktor_animation_ClothEntityFactory_H
