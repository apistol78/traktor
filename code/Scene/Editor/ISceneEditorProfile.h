#ifndef traktor_scene_ISceneEditorProfile_H
#define traktor_scene_ISceneEditorProfile_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Command;

		namespace custom
		{

class ToolBar;

		}
	}

	namespace render
	{

class IRenderView;
class PrimitiveRenderer;

	}

	namespace resource
	{

class IResourceFactory;

	}

	namespace world
	{

class WorldRenderer;
class IEntityFactory;
class IEntityRenderer;

	}

	namespace scene
	{

class SceneEditorContext;
class IEntityEditor;

/*! \brief Scene editor profile.
 * \ingroup Scene
 *
 * Profiles are instantiated automatically by the
 * scene editor and are used to create necessary
 * factories.
 */
class T_DLLCLASS ISceneEditorProfile : public Object
{
	T_RTTI_CLASS(ISceneEditorProfile)

public:
	/*! \brief Get supported set of entities.
	 *
	 * \return Type set of EntityData types.
	 */
	virtual TypeSet getEntityDataTypes() const = 0;

	/*! \brief Get UI commands.
	 *
	 * \param outCommands Output list of commands.
	 */
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const = 0;

	/*! \brief Create custom toolbar items.
	 *
	 * \param toolBar ToolBar widget.
	 */
	virtual void createToolBarItems(
		ui::custom::ToolBar* toolBar
	) const = 0;

	/*! \brief Create resource factories.
	 *
	 * \param context Scene editor context.
	 * \param outResourceFactories Output array of resource factories.
	 */
	virtual void createResourceFactories(
		SceneEditorContext* context,
		RefArray< resource::IResourceFactory >& outResourceFactories
	) const = 0;

	/*! \brief Create entity factories.
	 *
	 * \param context Scene editor context.
	 * \param outEntityFactories Output array of entity factories.
	 */
	virtual void createEntityFactories(
		SceneEditorContext* context,
		RefArray< world::IEntityFactory >& outEntityFactories
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

	/*! \brief Create entity editor factories.
	 *
	 * \param context Scene editor context.
	 * \param entityDataType Type of entity data.
	 * \return Entity editor instance.
	 */
	virtual IEntityEditor* createEntityEditor(
		SceneEditorContext* context,
		const Type& entityDataType
	) const = 0;
};

	}
}

#endif	// traktor_scene_ISceneEditorProfile_H
