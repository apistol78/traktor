#include "Animation/Editor/AnimatedMeshComponentEditor.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
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

	if (m_context->shouldDrawGuide(L"Animation.Skeleton"))
	{
		primitiveRenderer->pushWorld(m_entityAdapter->getTransform().toMatrix44());
		primitiveRenderer->pushDepthState(false, false, false);

		if (animatedMeshComponent)
		{
			const resource::Proxy< Skeleton >& skeleton = animatedMeshComponent->getSkeleton();

#if 0
			// Draw bind skeleton.
			const auto& jointTransforms = animatedEntity->getJointTransforms();
			if (jointTransforms.size() == skeleton->getJointCount())
			{
				const Color4ub color(0, 255, 0, 255);
				for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
				{
					const Joint* joint = skeleton->getJoint(i);
					primitiveRenderer->drawWireFrame(jointTransforms[i].toMatrix44(), joint->getRadius() * 4.0f);
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
#endif

			// Draw current pose.
			AlignedVector< Transform > poseTransforms = animatedMeshComponent->getPoseTransforms();
			if (poseTransforms.size() == skeleton->getJointCount())
			{
				const Color4ub color(255, 255, 0, 255);
				for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
				{
					const Joint* joint = skeleton->getJoint(i);
					primitiveRenderer->drawWireFrame(poseTransforms[i].toMatrix44(), joint->getRadius() * 4.0f);
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
				}
			}
		}

		primitiveRenderer->popDepthState();
		primitiveRenderer->popWorld();
	}

	scene::DefaultComponentEditor::drawGuide(primitiveRenderer);
}

	}
}
