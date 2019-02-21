#include "Animation/Editor/AnimatedMeshEntityEditor.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshEntityEditor", AnimatedMeshEntityEditor, scene::DefaultEntityEditor)

AnimatedMeshEntityEditor::AnimatedMeshEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
	updateSettings();
}

bool AnimatedMeshEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
		updateSettings();

	return scene::DefaultEntityEditor::handleCommand(command);
}

void AnimatedMeshEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const AnimatedMeshEntityData* animatedEntityData = checked_type_cast< const AnimatedMeshEntityData* >(getEntityAdapter()->getEntityData());
	/*const*/ AnimatedMeshEntity* animatedEntity = dynamic_type_cast< /*const*/ AnimatedMeshEntity* >(getEntityAdapter()->getEntity());

	if (getContext()->shouldDrawGuide(L"Animation.Skeleton"))
	{
		primitiveRenderer->pushWorld(getEntityAdapter()->getTransform().toMatrix44());
		primitiveRenderer->pushDepthState(false, false, false);

		if (animatedEntity)
		{
			const resource::Proxy< Skeleton >& skeleton = animatedEntity->getSkeleton();

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
						T_ASSERT (parent != nullptr);

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
			AlignedVector< Transform > poseTransforms = animatedEntity->getPoseTransforms();
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
						T_ASSERT (parent != nullptr);

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

	scene::DefaultEntityEditor::drawGuide(primitiveRenderer);
}

void AnimatedMeshEntityEditor::updateSettings()
{
	m_colorBone = getContext()->getEditor()->getSettings()->getProperty< Color4ub >(L"Editor.Colors/BoneWire");
}

	}
}
