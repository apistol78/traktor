#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureAssetEditor.h"
#include "Render/Editor/Texture/TextureAssetEditorFactory.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureAssetEditorFactory", 0, TextureAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet TextureAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TextureAsset >());
	return typeSet;
}

bool TextureAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo) const
{
	return true;
}

Ref< editor::IObjectEditor > TextureAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new TextureAssetEditor(editor);
}

void TextureAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
