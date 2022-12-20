/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/AnimatedMeshComponentEditor.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Physics/BallJoint.h"
#include "Physics/Body.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponentEditor", AnimatedMeshComponentEditor, scene::DefaultComponentEditor)

AnimatedMeshComponentEditor::AnimatedMeshComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	scene::DefaultComponentEditor(context, entityAdapter, componentData)
{
}

void AnimatedMeshComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const AnimatedMeshComponentData* animatedMeshComponentData = checked_type_cast< const AnimatedMeshComponentData* >(m_componentData);
	const AnimatedMeshComponent* animatedMeshComponent = dynamic_type_cast< const AnimatedMeshComponent* >(m_entityAdapter->getComponent< AnimatedMeshComponent >());

	if (
		m_context->shouldDrawGuide(L"Animation.Skeleton.Bind") ||
		m_context->shouldDrawGuide(L"Animation.Skeleton.Pose")
	)
	{
		if (animatedMeshComponent)
		{
			const resource::Proxy< Skeleton >& skeleton = animatedMeshComponent->getSkeleton();
			const auto& jointTransforms = animatedMeshComponent->getJointTransforms();

			// Draw pose controllers.
			primitiveRenderer->pushWorld(Matrix44::identity());
			primitiveRenderer->pushDepthState(false, false, false);

			if (auto ragDollPoseController = dynamic_type_cast< const RagDollPoseController* >(animatedMeshComponent->getPoseController()))
			{
				const auto& limbs = ragDollPoseController->getLimbs();

				for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
				{
					const Joint* joint = skeleton->getJoint(i);
					T_ASSERT(joint);

					const int32_t parent = joint->getParent();
					if (parent < 0)
						continue;

					Vector4 start = jointTransforms[parent].translation();
					Vector4 end = jointTransforms[i].translation();
					Scalar length = (end - start).length();

					if (length < joint->getRadius() * 2.0f)
						continue;

					if (!limbs[i])
						continue;

					Transform limbTransform[] { limbs[i]->getTransform(), limbs[i]->getTransform() };

					physics::CapsuleShapeDesc shapeDesc;
					shapeDesc.setRadius(joint->getRadius());
					shapeDesc.setLength(length);

					m_physicsRenderer.draw(
						m_context->getResourceManager(),
						primitiveRenderer,
						limbTransform,
						&shapeDesc
					);
				}

				for (auto joint : ragDollPoseController->getJoints())
				{
					primitiveRenderer->drawLine(
						joint->getBody1()->getTransform().translation().xyz1(),
						joint->getBody2()->getTransform().translation().xyz1(),
						Color4ub(0, 0, 255, 255)
					);

					if (auto ballJoint = dynamic_type_cast< const physics::BallJoint* >(joint))
					{
						primitiveRenderer->drawSolidPoint(ballJoint->getAnchor(), 8.0f, Color4ub(0, 0, 255, 255));
					}
				}
			}

			primitiveRenderer->popDepthState();
			primitiveRenderer->popWorld();

			primitiveRenderer->pushWorld(m_entityAdapter->getTransform().toMatrix44());
			primitiveRenderer->pushDepthState(false, false, false);

			if (m_context->shouldDrawGuide(L"Animation.Skeleton.Bind"))
			{
				// Draw bind skeleton.
				const auto& jointTransforms = animatedMeshComponent->getJointTransforms();
				if (jointTransforms.size() == skeleton->getJointCount())
				{
					const Color4ub color(0, 255, 0, 255);
					for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
					{
						const Joint* joint = skeleton->getJoint(i);
						primitiveRenderer->drawWireFrame(jointTransforms[i].toMatrix44(), joint->getRadius() * 1.0f);
						if (joint->getParent() >= 0)
						{
							const Joint* parent = skeleton->getJoint(joint->getParent());
							T_ASSERT(parent != nullptr);

							primitiveRenderer->drawLine(
								jointTransforms[joint->getParent()].translation(),
								jointTransforms[i].translation(),
								2.0f,
								color
							);
						}
					}
				}
			}

			if (m_context->shouldDrawGuide(L"Animation.Skeleton.Pose"))
			{
				const bool bindVisible = m_context->shouldDrawGuide(L"Animation.Skeleton.Bind");

				// Draw current pose.
				AlignedVector< Transform > poseTransforms = animatedMeshComponent->getPoseTransforms();
				if (poseTransforms.size() == skeleton->getJointCount())
				{
					const Color4ub color(255, 255, 0, 255);
					const Color4ub colorAlpha(255, 255, 0, 140);

					for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
					{
						const Joint* joint = skeleton->getJoint(i);

						if (bindVisible)
						{
							primitiveRenderer->drawLine(
								jointTransforms[i].translation(),
								poseTransforms[i].translation(),
								colorAlpha
							);
						}

						primitiveRenderer->drawWireFrame(poseTransforms[i].toMatrix44(), joint->getRadius() * 1.0f);

						if (joint->getParent() >= 0)
						{
							const Joint* parent = skeleton->getJoint(joint->getParent());
							T_ASSERT(parent != nullptr);

							primitiveRenderer->drawLine(
								poseTransforms[joint->getParent()].translation(),
								poseTransforms[i].translation(),
								2.0f,
								color
							);
						}

						primitiveRenderer->pushWorld(Matrix44::identity());
						primitiveRenderer->drawText(
							m_entityAdapter->getTransform() * poseTransforms[i].translation().xyz1(),
							0.4f,
							0.4f,
							joint->getName(),
							Color4ub(255, 255, 255, 255)
						);
						primitiveRenderer->popWorld();
					}
				}
			}

			primitiveRenderer->popDepthState();
			primitiveRenderer->popWorld();
		}
	}

	scene::DefaultComponentEditor::drawGuide(primitiveRenderer);
}

	}
}
