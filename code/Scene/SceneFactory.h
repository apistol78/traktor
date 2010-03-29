#ifndef traktor_scene_SceneFactory_H
#define traktor_scene_SceneFactory_H

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
	namespace db
	{

class Database;

	}

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

class T_DLLCLASS SceneFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	/*! \brief Construct scene factory.
	 *
	 * \param database Database
	 * \param renderSystem Render system.
	 * \param entityBuilder Entity builder.
	 * \param shadowQuality Max shadow quality level.
	 */
	SceneFactory(
		db::Database* database,
		render::IRenderSystem* renderSystem,
		world::IEntityBuilder* entityBuilder,
		world::WorldRenderSettings::ShadowQuality shadowQuality
	);

	void setShadowQuality(world::WorldRenderSettings::ShadowQuality shadowQuality);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_database;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< world::IEntityBuilder > m_entityBuilder;
	world::WorldRenderSettings::ShadowQuality m_shadowQuality;
};

	}
}

#endif	// traktor_scene_SceneFactory_H
