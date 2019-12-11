#pragma once

#include "Resource/IResourceFactory.h"
#include "World/WorldRenderSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
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

	namespace world
	{

class IEntityBuilder;

	}

	namespace scene
	{

/*! Scene factory.
 * \ingroup Scene
 */
class T_DLLCLASS SceneFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	/*! Construct scene factory.
	 *
	 * \param renderSystem Render system.
	 * \param entityBuilder Entity builder.
	 */
	SceneFactory(
		render::IRenderSystem* renderSystem,
		world::IEntityBuilder* entityBuilder
	);

	virtual const TypeInfoSet getResourceTypes() const override final;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const override final;

	virtual bool isCacheable(const TypeInfo& productType) const override final;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const override final;

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< world::IEntityBuilder > m_entityBuilder;
};

	}
}

