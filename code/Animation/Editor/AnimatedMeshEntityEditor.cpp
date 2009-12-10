#include "Animation/Editor/AnimatedMeshEntityEditor.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Bone.h"
#include "Animation/IPoseController.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshEntityEditor", AnimatedMeshEntityEditor, scene::DefaultEntityEditor)

AnimatedMeshEntityEditor::AnimatedMeshEntityEditor(scene::SceneEditorContext* context)
:	scene::DefaultEntityEditor(context)
{
	updateSettings(context);
}

bool AnimatedMeshEntityEditor::handleCommand(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const ui::Command& command
)
{
	if (command == L"Editor.SettingsChanged")
		updateSettings(context);

	return scene::DefaultEntityEditor::handleCommand(
		context,
		entityAdapter,
		command
	);
}

void AnimatedMeshEntityEditor::drawGuide(
	scene::SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	scene::EntityAdapter* entityAdapter
) const
{
	const AnimatedMeshEntityData* animatedEntityData = checked_type_cast< const AnimatedMeshEntityData* >(entityAdapter->getRealEntityData());
	const AnimatedMeshEntity* animatedEntity = checked_type_cast< const AnimatedMeshEntity* >(entityAdapter->getEntity());

	if (context->getGuideEnable())
	{
		primitiveRenderer->pushWorld(entityAdapter->getTransform().toMatrix44());
		primitiveRenderer->pushDepthEnable(false);

		resource::Proxy< Skeleton > skeleton = animatedEntityData->getSkeleton();
		if (skeleton.valid())
		{
			AlignedVector< Transform > poseTransforms = animatedEntity->getPoseTransforms();
			if (poseTransforms.empty())
				calculateBoneTransforms(skeleton, poseTransforms);

			if (poseTransforms.size() == skeleton->getBoneCount())
			{
				for (uint32_t i = 0; i < skeleton->getBoneCount(); ++i)
				{
					const Bone* bone = skeleton->getBone(i);

					Vector4 start = poseTransforms[i].translation();
					Vector4 end = poseTransforms[i].translation() + poseTransforms[i] * Vector4(0.0f, 0.0f, bone->getLength(), 0.0f);

					Vector4 d = poseTransforms[i].axisZ();
					Vector4 a = poseTransforms[i].axisX();
					Vector4 b = poseTransforms[i].axisY();

					Scalar radius = bone->getRadius();
					d *= radius;
					a *= radius;
					b *= radius;

					primitiveRenderer->drawLine(start, start + d + a + b, m_colorBone);
					primitiveRenderer->drawLine(start, start + d - a + b, m_colorBone);
					primitiveRenderer->drawLine(start, start + d + a - b, m_colorBone);
					primitiveRenderer->drawLine(start, start + d - a - b, m_colorBone);

					primitiveRenderer->drawLine(start + d + a + b, end, m_colorBone);
					primitiveRenderer->drawLine(start + d - a + b, end, m_colorBone);
					primitiveRenderer->drawLine(start + d + a - b, end, m_colorBone);
					primitiveRenderer->drawLine(start + d - a - b, end, m_colorBone);

					primitiveRenderer->drawLine(start + d + a + b, start + d - a + b, m_colorBone);
					primitiveRenderer->drawLine(start + d - a + b, start + d - a - b, m_colorBone);
					primitiveRenderer->drawLine(start + d - a - b, start + d + a - b, m_colorBone);
					primitiveRenderer->drawLine(start + d + a - b, start + d + a + b, m_colorBone);

					primitiveRenderer->drawLine(start, end, m_colorBone);
					primitiveRenderer->drawLine(start, start + a * Scalar(2.0f), Color(255, 0, 0, 180));
					primitiveRenderer->drawLine(start, start + b * Scalar(2.0f), Color(0, 255, 0, 180));
				}
			}
		}

		primitiveRenderer->popDepthEnable();
		primitiveRenderer->popWorld();
	}

	scene::DefaultEntityEditor::drawGuide(context, primitiveRenderer, entityAdapter);
}

void AnimatedMeshEntityEditor::updateSettings(scene::SceneEditorContext* context)
{
	Ref< editor::PropertyGroup > colors = context->getEditor()->getSettings()->getProperty< editor::PropertyGroup >(L"Editor.Colors");
	m_colorBone = colors->getProperty< editor::PropertyColor >(L"BoneWire");
}

	}
}
