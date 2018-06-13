#include "UiKit/Editor/WidgetPreviewEditor.h"
#include "UiKit/Editor/WidgetPreviewEditorFactory.h"
#include "UiKit/Editor/WidgetScaffolding.h"

namespace traktor
{
	namespace uikit
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.uikit.WidgetPreviewEditorFactory", 0, WidgetPreviewEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet WidgetPreviewEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< WidgetScaffolding >();
}

bool WidgetPreviewEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IObjectEditor > WidgetPreviewEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new WidgetPreviewEditor(editor);
}

void WidgetPreviewEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}	
}
