#include "Animation/Editor/AnimationEntityEditorFactory.h"
#include "Animation/Editor/AnimatedMeshEntityEditor.h"
#include "Animation/Editor/PathEntity/PathEntityEditor.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/PathEntity/PathEntityData.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationEntityEditorFactory", AnimationEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet AnimationEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AnimatedMeshEntityData >());
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > AnimationEntityEditorFactory::createEntityEditor(
	scene::SceneEditorContext* context,
	const TypeInfo& entityDataType
) const
{
	if (is_type_of< AnimatedMeshEntityData >(entityDataType))
		return new AnimatedMeshEntityEditor();
	if (is_type_of< PathEntityData >(entityDataType))
		return new PathEntityEditor();
	return 0;
}

	}
}
