#include "Render/Image/ImageProcessData.h"
#include "Render/Editor/Image/ImageProcessEditor.h"
#include "Render/Editor/Image/ImageProcessEditorFactory.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessEditorFactory", 0, ImageProcessEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet ImageProcessEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< ImageProcessData >();
	return typeSet;
}

bool ImageProcessEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > ImageProcessEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new ImageProcessEditor(editor);
}

void ImageProcessEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
