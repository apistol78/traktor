/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Math/TransformPath.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Bounding volume component for spatial queries.
 * \ingroup World
 */
class T_DLLCLASS PathComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit PathComponent(const TransformPath& path);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	Transform evaluate(float at) const;

	Transform evaluateDirectional(float at) const;

	float estimateLength() const;

	float findClosest(const Vector4& position) const;

private:
	TransformPath m_path;
	bool m_closed;
};

}
