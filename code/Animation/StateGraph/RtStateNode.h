/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class IPoseController;
class Pose;
class StateContext;
class Skeleton;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateNode : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual bool prepare(StateContext& outContext) const = 0;

	virtual void reset(
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& poseTransforms) const = 0;

	virtual void evaluate(
		StateContext& context,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		Pose& outPose,
		Vector4* outRootMotion) const = 0;

	virtual IPoseController* getActivePoseController() const = 0;

	virtual void getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const = 0;
};

}
