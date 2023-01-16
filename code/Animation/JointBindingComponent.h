/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS JointBindingComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	struct Binding
	{
		render::handle_t jointHandle;
		Ref< world::Entity > entity;
	};

	explicit JointBindingComponent(const AlignedVector< Binding >& bindings);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	/*! Get all joint entity bindings. */
	const AlignedVector< Binding >& getBindings() const { return m_bindings; }

private:
	AlignedVector< Binding > m_bindings;
	world::Entity* m_owner = nullptr;
	Transform m_transform;
};

}
