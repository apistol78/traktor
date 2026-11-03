/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/StateGraph/RtStateNodeData.h"
#include "Resource/Member.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class IPoseControllerData;
class ITransformTimeData;
class Skeleton;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateNodePoseControllerData : public RtStateNodeData
{
	T_RTTI_CLASS;

public:
	virtual Ref< RtStateNode > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform,
		bool editor) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StateGraphCompiler;

	Ref< const IPoseControllerData > m_poseController;
	Ref< const ITransformTimeData > m_transformTime;
};

}
