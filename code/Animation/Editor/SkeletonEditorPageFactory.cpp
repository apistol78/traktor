#include "Animation/Editor/SkeletonEditorPageFactory.h"
#include "Animation/Editor/SkeletonEditorPage.h"
#include "Animation/Skeleton.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SkeletonEditorPageFactory", SkeletonEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SkeletonEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Skeleton >());
	return typeSet;
}

Ref< editor::IEditorPage > SkeletonEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return new SkeletonEditorPage(editor);
}

void SkeletonEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Skeleton.Editor.AddBone"));
}

	}
}
