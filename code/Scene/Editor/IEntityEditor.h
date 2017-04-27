/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_IEntityEditor_H
#define traktor_scene_IEntityEditor_H

#include "Core/Object.h"
#include "Core/Math/Frustum.h"
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

class EntityAdapter;

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
	/*! \brief Is entity pickable.
	 *
	 * \return True if pickable.
	 */
	virtual bool isPickable() const = 0;

	/*! \brief Is entity a group?
	 *
	 * \return True if group.
	 */
	virtual bool isGroup() const = 0;

	/*! \brief Is entity children private.
	 *
	 * \return True if children private.
	 */
	virtual bool isChildrenPrivate() const = 0;

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

	/*! \brief Local frustum query.
	 *
	 * Query intersection of frustum and entity.
	 *
	 * \param worldFrustum Frustum in world space.
	 * \return True if intersection.
	 */
	virtual bool queryFrustum(const Frustum& worldFrustum) const = 0;

	/*! \brief Entity selected.
	 *
	 * \param selected True if entity was selected, false if it was deselected.
	 */
	virtual void entitySelected(bool selected) = 0;

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
