#ifndef traktor_scene_IEntityEditor_H
#define traktor_scene_IEntityEditor_H

#include "Core/Object.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
 *
 * Each created entity adapter gets a unique instance of
 * an entity editor thus it's safe to store
 * editing specific data as members of this class.
 */
class T_DLLCLASS IEntityEditor : public Object
{
	T_RTTI_CLASS;

public:
	struct ApplyParams
	{
		Matrix44 viewTransform;
		Vector4 screenDelta;
		Vector4 viewDelta;
		Vector4 worldDelta;
		Vector4 worldRayOrigin;
		Vector4 worldRayDirection;
		int mouseButton;
	};

	/*! \brief Is entity a group?
	 *
	 * \param entityAdapter Entity adapter.
	 * \return True if group.
	 */
	virtual bool isGroup() const = 0;

	/*! \brief Add child entity.
	 *
	 * \param childEntityAdapter Child entity.
	 * \return True if successfully added child.
	 */
	virtual bool addChildEntity(EntityAdapter* childEntityAdapter) const = 0;

	/*! \brief Remove child entity.
	 *
	 * \param childEntityAdapter Child entity.
	 * \return True if successfully removed child.
	 */
	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const = 0;

	/*! \brief Local ray query.
	 *
	 * Query closest intersection of ray and entity.
	 *
	 * \param worldRayOrigin Ray origin in world space.
	 * \param worldRayDirection Ray direction in world space.
	 * \param outDistance Distance to closest intersection from ray origin.
	 * \return True if intersection found.
	 */
	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const = 0;

	/*! \brief Entity selected.
	 *
	 * \param selected True if entity was selected, false if it was deselected.
	 */
	virtual void entitySelected(bool selected) = 0;

	/*! \brief Cursor moved, not in modification state..
	 */
	virtual void cursorMoved(const ApplyParams& params) = 0;

	/*! \brief Begin modifier on entity.
	 */
	virtual void beginModifier(const ApplyParams& params) = 0;

	/*! \brief Apply modifier on entity.
	 *
	 * \param params User parameters.
	 */
	virtual void applyModifier(const ApplyParams& params) = 0;

	/*! \brief Begin modifier on entity.
	 */
	virtual void endModifier(const ApplyParams& params) = 0;

	/*! \brief Handle shortcut.
	 *
	 * \param command Shortcut command.
	 */
	virtual bool handleCommand(const ui::Command& command) = 0;

	/*! \brief Draw guide for entity.
	 *
	 * A guide is the wire overlay
	 * in the 3d editor view.
	 *
	 * \param primitiveRenderer Primitive wire renderer.
	 */
	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const = 0;

	/*! \brief Get status text.
	 *
	 * Status text is shown beneath the
	 * 3d editor view.
	 *
	 * \param outStatusText Status text.
	 * \return If status text available.
	 */
	virtual bool getStatusText(std::wstring& outStatusText) const = 0;
};

	}
}

#endif	// traktor_scene_IEntityEditor_H
