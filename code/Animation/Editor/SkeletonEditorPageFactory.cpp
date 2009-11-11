#include "Animation/Editor/SkeletonEditorPageFactory.h"
#include "Animation/Editor/SkeletonEditorPage.h"
#include "Animation/Skeleton.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.SkeletonEditorPageFactory", SkeletonEditorPageFactory, editor::IEditorPageFactory)

const TypeSet SkeletonEditorPageFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Skeleton >());
	return typeSet;
}

Ref< editor::IEditorPage > SkeletonEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return gc_new< SkeletonEditorPage >(editor);
}

void SkeletonEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Skeleton.Editor.AddBone"));
}

	}
}
