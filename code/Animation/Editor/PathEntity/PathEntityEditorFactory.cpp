#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/Editor/PathEntity/PathEntityEditor.h"
#include "Animation/Editor/PathEntity/PathEntityEditorFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityEditorFactory", PathEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet PathEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< PathEntityData >();
}

Ref< scene::IEntityEditor > PathEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new PathEntityEditor(context, entityAdapter);
}

	}
}
