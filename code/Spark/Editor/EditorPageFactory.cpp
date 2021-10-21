#include "Core/Serialization/DeepClone.h"
#include "Spark/Editor/EditorPage.h"
#include "Spark/Editor/EditorPageFactory.h"
#include "Spark/Editor/MovieAsset.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.EditorPageFactory", 0, EditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet EditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< MovieAsset >();
}

bool EditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{14D6A2DB-796D-E54D-9D70-73DE4AE7C4E8}"));	// System/Flash/Shaders/FlashShaderAssets
	return true;
}

Ref< editor::IEditorPage > EditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new EditorPage(editor, site, document);
}

void EditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Flash.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Play"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Forward"));
}

Ref< ISerializable > EditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
