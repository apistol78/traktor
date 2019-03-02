#pragma once

#include "Editor/IObjectEditor.h"
#include "Ui/PropertyList/AutoPropertyList.h"

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

class PropertyCommandEvent;
class ToolBar;
class ToolBarButtonClickEvent;

	}

	namespace sound
	{

class SoundAsset;
class SoundChannel;
class SoundSystem;

class SoundAssetEditor
:	public editor::IObjectEditor
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	SoundAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< SoundAsset > m_asset;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::AutoPropertyList > m_propertyList;
	Ref< SoundSystem > m_soundSystem;
	Ref< SoundChannel > m_soundChannel;

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventPropertyCommand(ui::PropertyCommandEvent* event);

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const override final;
};

	}
}

