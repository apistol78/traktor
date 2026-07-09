/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/SkeletonComponentData.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Animation/Editor/AnimationComponentEditor.h"
#include "Animation/Editor/SkeletonRenderer.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Physics/BallJoint.h"
#include "Physics/Body.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationComponentEditor", AnimationComponentEditor, scene::DefaultComponentEditor)

AnimationComponentEditor::AnimationComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	scene::DefaultComponentEditor(context, entityAdapter, componentData)
{
}

void AnimationComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const SkeletonComponentData* skeletonComponentData = checked_type_cast< const SkeletonComponentData* >(m_componentData);
	const SkeletonComponent* skeletonComponent = dynamic_type_cast< const SkeletonComponent* >(m_entityAdapter->getComponent< SkeletonComponent >());

	if (skeletonComponent)
	{
		// Draw pose controllers.
		if (m_context->shouldDrawGuide(L"Animation.Controller"))
		{
			primitiveRenderer->pushWorld(Matrix44::identity());
			primitiveRenderer->pushDepthState(false, false, false);

			if (auto ragDollPoseController = dynamic_type_cast< const RagDollPoseController* >(skeletonComponent->getPoseController()))
			{
				// Draw each limb's collision shape at its current body transform.
				const RefArray< physics::Body >& limbs = ragDollPoseController->getLimbs();
				const RefArray< const physics::ShapeDesc >& limbShapes = ragDollPoseController->getLimbShapes();

				for (uint32_t i = 0; i < limbs.size(); ++i)
				{
					if (!limbs[i] || i >= limbShapes.size() || !limbShapes[i])
						continue;

					const Transform bodyTransform[] = { limbs[i]->getTransform(), limbs[i]->getTransform() };
					m_physicsRenderer.draw(
						m_context->getResourceManager(),
						primitiveRenderer,
						bodyTransform,
						limbShapes[i]
					);
				}

				// Draw constraint joints between connected limbs.
				for (auto joint : ragDollPoseController->getJoints())
				{
					if (!joint || !joint->getBody1() || !joint->getBody2())
						continue;

					primitiveRenderer->drawLine(
						joint->getBody1()->getTransform().translation().xyz1(),
						joint->getBody2()->getTransform().translation().xyz1(),
						Color4ub(0, 0, 255, 255)
					);

					if (auto ballJoint = dynamic_type_cast< const physics::BallJoint* >(joint))
						primitiveRenderer->drawSolidPoint(ballJoint->getAnchor().xyz1(), 8.0f, Color4ub(0, 0, 255, 255));
				}
			}

			primitiveRenderer->popDepthState();
			primitiveRenderer->popWorld();
		}

		drawSkeleton(
			primitiveRenderer,
			skeletonComponent,
			m_entityAdapter->getTransform().toMatrix44(),
			m_context->shouldDrawGuide(L"Animation.Skeleton.Bind"),
			m_context->shouldDrawGuide(L"Animation.Skeleton.Pose")
		);
	}

	scene::DefaultComponentEditor::drawGuide(primitiveRenderer);
}

}
