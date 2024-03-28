/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::shape
{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ControlPointComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit ControlPointComponent(float scale);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	bool checkDirty();

	const Transform& getTransform() const { return m_transform; }

	float getScale() const { return m_scale; }

private:
	bool m_dirty;
	Transform m_transform;
	float m_scale;
};

}
