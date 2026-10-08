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
#include "Core/Math/Vector4.h"
#include "World/IEntityComponentData.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class LookAtComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS LookAtComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< LookAtComponent > createComponent() const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_joint = L"Neck";	//!< Joint turned to look; its subtree follows.
	Vector4 m_eyeOffset = Vector4(0.0f, 0.15f, 0.1f, 0.0f);	//!< Eyes from the joint in the bind pose; object axes, y up and z forward.
	float m_maxYaw = 70.0f;			//!< Furthest turn either side of the chest's forward, in degrees.
	float m_maxPitch = 35.0f;		//!< Furthest tilt above and below the horizon, in degrees.
	float m_smoothTime = 0.15f;		//!< Settle time of the gaze.
	float m_cullDistance = 20.0f;	//!< No look at beyond this distance from the camera.
};

}
