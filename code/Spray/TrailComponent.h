/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Spray/Types.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IWorldRenderPass;
class WorldRenderView;

}

namespace traktor::spray
{

class TrailInstance;
class TrailRenderer;

/*! Ribbon trail component instance.
 * \ingroup Spray
 *
 * As long as the component is enabled points are automatically
 * added from the owner entity's transform; disable the component
 * in order to feed the trail with explicit points through \a addPoint.
 */
class T_DLLCLASS TrailComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit TrailComponent(TrailInstance* trailInstance);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void build(
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		TrailRenderer* trailRenderer) const;

	/*! Add point to trail.
	 *
	 * \param position Position of point, in world space.
	 */
	void addPoint(const Vector4& position);

	void setEnable(bool enable) { m_enable = enable; }

	bool isEnable() const { return m_enable; }

	TrailInstance* getTrailInstance() const { return m_trailInstance; }

private:
	Transform m_transform = Transform::identity();
	Ref< TrailInstance > m_trailInstance;
	Context m_context;
	bool m_enable = true;
};

}
