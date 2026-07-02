/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/SkeletonRenderer.h"

#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::animation
{

void drawSkeleton(
	render::PrimitiveRenderer* primitiveRenderer,
	const SkeletonComponent* skeletonComponent,
	const Matrix44& worldTransform,
	bool drawBind,
	bool drawPose
)
{
	const resource::Proxy< Skeleton >& skeleton = skeletonComponent->getSkeleton();
	if (!skeleton)
		return;

	const auto& jointTransforms = skeletonComponent->getJointTransforms();

	primitiveRenderer->pushWorld(worldTransform);
	primitiveRenderer->pushDepthState(false, false, false);

	if (drawBind)
	{
		if (jointTransforms.size() == skeleton->getJointCount())
		{
			const Color4ub color(0, 255, 0, 250);
			for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
			{
				const Joint* joint = skeleton->getJoint(i);

				primitiveRenderer->drawWireFrame(jointTransforms[i].toMatrix44(), joint->getRadius() * 1.0f);

				if (joint->getParent() >= 0)
				{
					const Joint* parent = skeleton->getJoint(joint->getParent());
					T_FATAL_ASSERT(parent != nullptr);

					const Vector4 f = jointTransforms[joint->getParent()].translation().xyz1();
					const Vector4 t = jointTransforms[i].translation().xyz1();

					const Scalar ln = min((t - f).length() * 0.8_simd, 0.1_simd);
					const Vector4 c = t - (t - f).normalized() * ln;

					primitiveRenderer->drawLine(
						f,
						c,
						1.0f,
						color
					);
					primitiveRenderer->drawArrowHead(
						c,
						t,
						0.8f,
						color
					);
				}
			}
		}
	}

	if (drawPose)
	{
		const AlignedVector< Transform >& poseTransforms = skeletonComponent->getPoseTransforms();
		if (poseTransforms.size() == skeleton->getJointCount())
		{
			const Color4ub color(255, 255, 0, 250);
			for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
			{
				const Joint* joint = skeleton->getJoint(i);

				primitiveRenderer->drawWireFrame(poseTransforms[i].toMatrix44(), joint->getRadius() * 1.0f);

				if (joint->getParent() >= 0)
				{
					const Joint* parent = skeleton->getJoint(joint->getParent());
					T_FATAL_ASSERT(parent != nullptr);

					const Vector4 f = poseTransforms[joint->getParent()].translation().xyz1();
					const Vector4 t = poseTransforms[i].translation().xyz1();

					const Scalar ln = min((t - f).length() * 0.8_simd, 0.1_simd);
					const Vector4 c = t - (t - f).normalized() * ln;

					primitiveRenderer->drawLine(
						f,
						c,
						1.0f,
						color
					);
					primitiveRenderer->drawArrowHead(
						c,
						t,
						0.8f,
						color
					);
				}
			}
		}
	}

	primitiveRenderer->popDepthState();
	primitiveRenderer->popWorld();
}

}
