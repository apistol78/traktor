#include "Animation/Editor/AnimationEditorPageFactory.h"
#include "Animation/Editor/AnimationEditorPage.h"
#include "Animation/Animation/Animation.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.AnimationEditorPageFactory", AnimationEditorPageFactory, editor::EditorPageFactory)

const TypeSet AnimationEditorPageFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Animation >());
	return typeSet;
}

editor::EditorPage* AnimationEditorPageFactory::createEditorPage(editor::Editor* editor) const
{
	return gc_new< AnimationEditorPage >(editor);
}

void AnimationEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Animation.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Play"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Forward"));
	outCommands.push_back(ui::Command(L"Animation.Editor.BrowseSkeleton"));
	outCommands.push_back(ui::Command(L"Animation.Editor.BrowseSkin"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleTrail"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleTwistLock"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleIK"));
	outCommands.push_back(ui::Command(L"Animation.Editor.SelectPreviousBone"));
	outCommands.push_back(ui::Command(L"Animation.Editor.SelectNextBone"));
}

	}
}
