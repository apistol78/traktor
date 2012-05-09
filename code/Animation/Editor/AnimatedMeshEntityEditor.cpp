#include "Animation/Editor/AnimatedMeshEntityEditor.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Bone.h"
#include "Animation/IPoseController.h"
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

	primitiveRenderer->pushWorld(getEntityAdapter()->getTransform().toMatrix44());
	primitiveRenderer->pushDepthEnable(false);

	if (animatedEntity)
	{
		const resource::Proxy< Skeleton >& skeleton = animatedEntity->getSkeleton();

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
				primitiveRenderer->drawLine(start, start + a * Scalar(2.0f), Color4ub(255, 0, 0, 180));
				primitiveRenderer->drawLine(start, start + b * Scalar(2.0f), Color4ub(0, 255, 0, 180));
			}
		}
	}

	primitiveRenderer->popDepthEnable();
	primitiveRenderer->popWorld();

	scene::DefaultEntityEditor::drawGuide(primitiveRenderer);
}

void AnimatedMeshEntityEditor::updateSettings()
{
	m_colorBone = getContext()->getEditor()->getSettings()->getProperty< PropertyColor >(L"Editor.Colors/BoneWire");
}

	}
}
