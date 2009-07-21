#ifndef traktor_scene_IEntityEditor_H
#define traktor_scene_IEntityEditor_H

#include "Core/Object.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Matrix44.h"

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

	}

	namespace render
	{

class PrimitiveRenderer;

	}

	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;
class IModifier;

/*! \brief Abstract entity editor class.
 *
 * The scene editor uses specializations of this
 * class to apply changes to selected entities.
 */
class T_DLLCLASS IEntityEditor : public Object
{
	T_RTTI_CLASS(IEntityEditor)

public:
	/*! \brief Get set of supported entities.
	 *
	 * \return Set of supported entity types.
	 */
	virtual TypeSet getEntityTypes() const = 0;

	/*! \brief Is entity pick-able?
	 *
	 * Return true if entity can be selected
	 * through the 3D preview.
	 *
	 * \param entityAdapter Entity adapter.
	 * \return True if pick-able.
	 */
	virtual bool isPickable(
		EntityAdapter* entityAdapter
	) const = 0;

	/*! \brief Entity selected.
	 *
	 * \param context Scene editor context.
	 * \param entityAdapter Selected entity adapter.
	 * \param selected True if entity was selected, false if it was deselected.
	 */
	virtual void entitySelected(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		bool selected
	) const = 0;

	/*! \brief Apply modifier on entity.
	 *
	 * \param context Scene editor context.
	 * \param entityAdapter Selected entity adapter.
	 * \param viewTransform User view transform.
	 * \param mouseDelta Delta movement of mouse.
	 * \param mouseButton Pressed mouse button.
	 */
	virtual void applyModifier(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		const Matrix44& viewTransform,
		const Vector2& mouseDelta,
		int mouseButton
	) const = 0;

	/*! \brief Handle shortcut.
	 *
	 * \param context Scene editor context.
	 * \param entityAdapter Selected entity adapter.
	 * \param command Shortcut command.
	 */
	virtual bool handleCommand(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		const ui::Command& command
	) const = 0;

	/*! \brief Draw guide for entity.
	 *
	 * A guide is the wire overlay
	 * in the 3d editor view.
	 *
	 * \param context Scene editor context.
	 * \param primitiveRenderer Primitive wire renderer.
	 * \param entityAdapter Entity adapter, not necessarily selected.
	 */
	virtual void drawGuide(
		SceneEditorContext* context,
		render::PrimitiveRenderer* primitiveRenderer,
		EntityAdapter* entityAdapter
	) const = 0;

	/*! \brief Get status text.
	 *
	 * Status text is shown beneath the
	 * 3d editor view.
	 *
	 * \param context Scene editor context.
	 * \param entityAdapter Selected entity adapter.
	 * \param outStatusText Status text.
	 * \return If status text available.
	 */
	virtual bool getStatusText(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		std::wstring& outStatusText
	) const = 0;
};

	}
}

#endif	// traktor_scene_IEntityEditor_H
