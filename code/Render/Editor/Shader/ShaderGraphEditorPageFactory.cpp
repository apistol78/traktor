#include "Render/Editor/Shader/ShaderGraphEditorPageFactory.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Shader/ShaderGraph.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderGraphEditorPageFactory", 0, ShaderGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ShaderGraphEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ShaderGraph >());
	return typeSet;
}

Ref< editor::IEditorPage > ShaderGraphEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return new ShaderGraphEditorPage(editor);
}

void ShaderGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.EventSpaceHorizontally"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.Center"));
}

	}
}
