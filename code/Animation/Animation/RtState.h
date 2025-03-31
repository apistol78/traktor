/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Animation;
class IPoseController;
class Pose;
class StateContext;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtState : public Object
{
	T_RTTI_CLASS;

public:
	bool prepare(StateContext& outContext) const;

	void evaluate(const StateContext& context, Pose& outPose) const;

private:
	friend class RtStateData;

	resource::Proxy< Animation > m_animation;
	Ref< IPoseController > m_poseController;
};

}
