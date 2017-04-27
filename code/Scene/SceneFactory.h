/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

/*! \brief Scene factory.
 * \ingroup Scene
 */
class T_DLLCLASS SceneFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	/*! \brief Construct scene factory.
	 *
	 * \param renderSystem Render system.
	 * \param entityBuilder Entity builder.
	 */
	SceneFactory(
		render::IRenderSystem* renderSystem,
		world::IEntityBuilder* entityBuilder
	);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< world::IEntityBuilder > m_entityBuilder;
};

	}
}

#endif	// traktor_scene_SceneFactory_H
