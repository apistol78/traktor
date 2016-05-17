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

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void apply() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferredSize() const T_OVERRIDE T_FINAL;

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

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_sound_SoundAssetEditor_H
