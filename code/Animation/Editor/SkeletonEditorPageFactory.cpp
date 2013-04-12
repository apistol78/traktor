#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonEditorPage.h"
#include "Animation/Editor/SkeletonEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SkeletonEditorPageFactory", 0, SkeletonEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SkeletonEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Skeleton >());
	return typeSet;
}

bool SkeletonEditorPageFactory::needOutputResources(const TypeInfo& typeInfo) const
{
	return false;
}

Ref< editor::IEditorPage > SkeletonEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new SkeletonEditorPage(editor, site, document);
}

void SkeletonEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Skeleton.Editor.AddJoint"));
}

	}
}
