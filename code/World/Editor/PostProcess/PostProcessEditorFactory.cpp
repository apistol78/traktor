#include "World/PostProcess/PostProcessSettings.h"
#include "World/Editor/PostProcess/PostProcessEditor.h"
#include "World/Editor/PostProcess/PostProcessEditorFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.PostProcessEditorFactory", 0, PostProcessEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet PostProcessEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PostProcessSettings >());
	return typeSet;
}

Ref< editor::IObjectEditor > PostProcessEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new PostProcessEditor(editor);
}

	}
}
