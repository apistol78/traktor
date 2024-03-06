/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Command;

}

namespace traktor::render
{

class PrimitiveRenderer;

}

namespace traktor::scene
{

class EntityAdapter;

/*! Abstract entity editor class.
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
	/*! Is entity pickable.
	 *
	 * \return True if pickable.
	 */
	virtual bool isPickable() const = 0;

	/*! Is entity a group?
	 *
	 * \return True if group.
	 */
	virtual bool isGroup() const = 0;

	/*! Is entity children private.
	 *
	 * \return True if children private.
	 */
	virtual bool isChildrenPrivate() const = 0;

	/*! Add child entity.
	 *
	 * \param childEntityAdapter Child entity.
	 * \return True if successfully added child.
	 */
	virtual bool addChildEntity(EntityAdapter* insertAfterEntityAdapter, EntityAdapter* childEntityAdapter) const = 0;

	/*! Remove child entity.
	 *
	 * \param childEntityAdapter Child entity.
	 * \return True if successfully removed child.
	 */
	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const = 0;

	/*! Local ray query.
	 *
	 * Query closest intersection of ray and entity.
	 *
	 * \param worldRayOrigin Ray origin in world space.
	 * \param worldRayDirection Ray direction in world space.
	 * \param outDistance Distance to closest intersection from ray origin.
	 * \return True if intersection found.
	 */
	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const = 0;

	/*! Local frustum query.
	 *
	 * Query intersection of frustum and entity.
	 *
	 * \param worldFrustum Frustum in world space.
	 * \return True if intersection.
	 */
	virtual bool queryFrustum(const Frustum& worldFrustum) const = 0;

	/*! Entity selected.
	 *
	 * \param selected True if entity was selected, false if it was deselected.
	 */
	virtual void entitySelected(bool selected) = 0;

	/*! Handle shortcut.
	 *
	 * \param command Shortcut command.
	 */
	virtual bool handleCommand(const ui::Command& command) = 0;

	/*! Draw guide for entity.
	 *
	 * A guide is the wire overlay
	 * in the 3d editor view.
	 *
	 * \param primitiveRenderer Primitive wire renderer.
	 */
	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer, const ui::Size& clientSize) const = 0;

	/*! Get status text.
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
