/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;

}

namespace traktor::animation
{

class Animation;
class IPoseController;
class RtStackLayer;
class Skeleton;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStack : public Object
{
	T_RTTI_CLASS;

public:
	struct Layer
	{
		// Data
		render::Handle name;
		resource::Proxy< Animation > animation;
		Ref< IPoseController > poseController;

		// Mutable
		bool active = false;
		float weight = 0.0f;

		void evaluate(
			float lastTime,
			float time,
			float deltaTime,
			const Transform& worldTransform,
			const Skeleton* skeleton,
			const AlignedVector< Transform >& jointTransforms,
			Pose& outPose,
			Vector4& outRootMotion) const;
	};

	explicit RtStack(const AlignedVector< Layer >& layers);

	void destroy();

	void setOwner(world::Entity* owner);

	void setTransform(const Transform& transform);

	void setActive(render::Handle name, bool active);

	bool isActive(render::Handle name) const;

	void activateOnly(render::Handle name);

	bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms);

	Vector4 consumeRootMotion();

	IPoseController* getActivePoseController() const;

	void getPoseControllersOf(const TypeInfo& type, bool activeOnly, RefArray< IPoseController >& outControllers) const;

private:
	AlignedVector< Layer > m_layers;
	Vector4 m_rootMotion = Vector4::zero();
	float m_lastTime = -1.0f;
};

}
