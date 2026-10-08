/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
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

class T_DLLCLASS LookAtComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	struct Settings
	{
		render::handle_t joint = 0;
		Vector4 eyeOffset = Vector4(0.0f, 0.15f, 0.1f, 0.0f);	//!< Eyes from the joint, in bind pose object axes.
		float maxYaw = 1.2f;			//!< Radians either side of the chest's forward.
		float maxPitch = 0.6f;			//!< Radians above and below the horizon.
		float smoothTime = 0.15f;
		float cullDistance = 20.0f;
	};

	explicit LookAtComponent(const Settings& settings);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	/*! Set point to look at, in world space. */
	void setTarget(const Vector4& target) { m_target = target.xyz1(); }

	/*! Get point to look at, in world space. */
	const Vector4& getTarget() const { return m_target; }

	/*! Enable or disable; the gaze blends in or out over the smooth time. */
	void setEnable(bool enable) { m_enable = enable; }

	/*! Check if enabled. */
	bool getEnable() const { return m_enable; }

private:
	world::Entity* m_owner = nullptr;
	Settings m_settings;
	Vector4 m_target = Vector4::origo();
	bool m_enable = false;
	float m_weight = 0.0f;			//!< Blend of the gaze over the animated head.
	float m_gazeYaw = 0.0f;			//!< Smoothed gaze yaw, in world space.
	float m_gazePitch = 0.0f;		//!< Smoothed gaze pitch, in world space.
	int32_t m_revision = -1;		//!< Skeleton pose revision written last.
};

}
