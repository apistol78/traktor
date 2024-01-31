/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;

/*! Entity component.
 * \ingroup World
 */
class T_DLLCLASS IEntityComponent : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! Set owner entity of component.
	 * \param owner New component owner.
	 */
	virtual void setOwner(Entity* owner) = 0;

	/*! Called when entity's transform is modified.
	 * \param transform New transform.
	 */
	virtual void setTransform(const Transform& transform) = 0;

	/*! Called when entity's state is modified.
	 * \param state Entity state.
	 */
	virtual void setState(uint32_t state) {}

	/*! Calculate bounding box of this component.
	 * \return Bounding box.
	 */
	virtual Aabb3 getBoundingBox() const = 0;

	/*! Update component
	 * \param update Update information.
	 */
	virtual void update(const UpdateParams& update) = 0;
};

}
