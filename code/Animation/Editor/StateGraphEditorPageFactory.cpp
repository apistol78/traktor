#include "Animation/Editor/StateGraphEditorPageFactory.h"
#include "Animation/Editor/StateGraphEditorPage.h"
#include "Animation/Animation/StateGraph.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.StateGraphEditorPageFactory", StateGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeSet StateGraphEditorPageFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< StateGraph >());
	return typeSet;
}

editor::IEditorPage* StateGraphEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return gc_new< StateGraphEditorPage >(editor);
}

void StateGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.EventSpaceHorizontally"));
}

	}
}
