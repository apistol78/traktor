#ifndef traktor_sound_SoundAssetEditor_H
#define traktor_sound_SoundAssetEditor_H

#include "Editor/IObjectEditor.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
		namespace custom
		{

class PropertyCommandEvent;
class ToolBar;
class ToolBarButtonClickEvent;

		}
	}

	namespace sound
	{

class SoundAsset;
class SoundChannel;
class SoundSystem;

class SoundAssetEditor
:	public editor::IObjectEditor
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	SoundAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< SoundAsset > m_asset;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::AutoPropertyList > m_propertyList;
	Ref< SoundSystem > m_soundSystem;
	Ref< SoundChannel > m_soundChannel;

	void eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventPropertyCommand(ui::custom::PropertyCommandEvent* event);

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;
};

	}
}

#endif	// traktor_sound_SoundAssetEditor_H
