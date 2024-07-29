/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IObjectEditor.h"
#include "Ui/PropertyList/AutoPropertyList.h"

namespace traktor::db
{

class Instance;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class PropertyCommandEvent;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::sound
{

class SoundHandle;
class SoundPlayer; 
class SoundAsset;

class SoundAssetEditor
:	public editor::IObjectEditor
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	explicit SoundAssetEditor(editor::IEditor* editor);

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
	Ref< SoundPlayer > m_soundPlayer;
	Ref< SoundHandle > m_soundHandle;

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventPropertyCommand(ui::PropertyCommandEvent* event);

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const override final;
};

}
