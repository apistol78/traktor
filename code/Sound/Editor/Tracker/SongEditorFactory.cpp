#include "Sound/Editor/Tracker/SongAsset.h"
#include "Sound/Editor/Tracker/SongEditor.h"
#include "Sound/Editor/Tracker/SongEditorFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SongEditorFactory", 0, SongEditorFactory, editor::IEditorPageFactory)

const TypeInfoSet SongEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< SongAsset >();
	return typeSet;
}

bool SongEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IEditorPage > SongEditorFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new SongEditor(editor, site, document);
}

void SongEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
