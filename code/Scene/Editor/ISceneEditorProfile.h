/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_ISceneEditorProfile_H
#define traktor_scene_ISceneEditorProfile_H

#include <list>
#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace render
	{

struct DebugTarget;
class IRenderView;
class PrimitiveRenderer;

	}

	namespace resource
	{

class IResourceFactory;

	}

	namespace ui
	{

class Command;

	}

	namespace world
	{

class EntityData;
class IEntityFactory;
class IEntityRenderer;

	}

	namespace scene
	{

class SceneEditorContext;
class IComponentEditorFactory;
class ISceneEditorPlugin;
class ISceneControllerEditorFactory;
class IEntityEditorFactory;

/*! \brief Scene editor profile.
 * \ingroup Scene
 *
 * Profiles are instantiated automatically by the
 * scene editor and are used to create necessary
 * factories.
 */
class T_DLLCLASS ISceneEditorProfile : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get UI commands.
	 *
	 * \param outCommands Output list of commands.
	 */
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const = 0;

	/*! \brief Get guide draw ids.
	 */
	virtual void getGuideDrawIds(
		std::set< std::wstring >& outIds
	) const = 0;

	/*! \brief Create plugins.
	 *
	 * \param context Scene editor context.
	 * \param outEditorPlugins Editor plugins.
	 */
	virtual void createEditorPlugins(
		SceneEditorContext* context,
		RefArray< ISceneEditorPlugin >& outEditorPlugins
	) const = 0;

	/*! \brief Create resource factories.
	 *
	 * \param context Scene editor context.
	 * \param outResourceFactories Output array of resource factories.
	 */
	virtual void createResourceFactories(
		SceneEditorContext* context,
		RefArray< const resource::IResourceFactory >& outResourceFactories
	) const = 0;

	/*! \brief Create entity factories.
	 *
	 * \param context Scene editor context.
	 * \param outEntityFactories Output array of entity factories.
	 */
	virtual void createEntityFactories(
		SceneEditorContext* context,
		RefArray< const world::IEntityFactory >& outEntityFactories
	) const = 0;

	/*! \brief Create entity renderers.
	 *
	 * \param context Scene editor context.
	 * \param renderView Scene editor render view.
	 * \param primitiveRenderer Primitive renderer.
	 * \param outEntityRenderers Output array of entity renderers.
	 */
	virtual void createEntityRenderers(
		SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const = 0;

	/*! \brief Create scene controller editor factories.
	 *
	 * \param context Scene editor context.
	 * \param outControllerEditorFactories Scene controller editor factories.
	 */
	virtual void createControllerEditorFactories(
		SceneEditorContext* context,
		RefArray< const ISceneControllerEditorFactory >& outControllerEditorFactories
	) const = 0;

	/*! \brief Create entity editor factories.
	 *
	 * \param context Scene editor context.
	 * \param outEntityEditorFactories Entity editor factories.
	 */
	virtual void createEntityEditorFactories(
		SceneEditorContext* context,
		RefArray< const IEntityEditorFactory >& outEntityEditorFactories
	) const = 0;

	/*! \brief Create entity component editor factories.
	 *
	 * \param context Scene editor context.
	 * \param outComponentEditorFactories Component editor factories.
	 */
	virtual void createComponentEditorFactories(
		SceneEditorContext* context,
		RefArray< const IComponentEditorFactory >& outComponentEditorFactories
	) const = 0;

	/*! \brief Create entity from database instance.
	 *
	 * \param instance Database instance.
	 * \return Entity data from instance.
	 */
	virtual Ref< world::EntityData > createEntityData(
		SceneEditorContext* context,
		db::Instance* instance
	) const = 0;

	/*! \brief Get debug targets.
	 */
	virtual void getDebugTargets(
		SceneEditorContext* context,
		std::vector< render::DebugTarget >& outDebugTargets
	) const = 0;
};

	}
}

#endif	// traktor_scene_ISceneEditorProfile_H
