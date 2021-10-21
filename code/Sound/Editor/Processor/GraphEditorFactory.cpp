#include "Core/Serialization/DeepClone.h"
#include "Sound/Editor/Processor/GraphAsset.h"
#include "Sound/Editor/Processor/GraphEditor.h"
#include "Sound/Editor/Processor/GraphEditorFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.GraphEditorFactory", 0, GraphEditorFactory, editor::IEditorPageFactory)

const TypeInfoSet GraphEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< GraphAsset >();
}

bool GraphEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IEditorPage > GraphEditorFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new GraphEditor(editor, site, document);
}

void GraphEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > GraphEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
