/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Editor/Deploy/Feature.h"
#include "Editor/IObjectEditor.h"

namespace traktor
{

class PropertyStringSet;

	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Edit;
class EditList;
class ListBox;

	}

	namespace runtime
	{

class FeatureEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	FeatureEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< Feature > m_feature;
	Ref< ui::Edit > m_editName;
	Ref< ui::Edit > m_editPriority;
	Ref< ui::ListBox > m_listPlatforms;
	Ref< ui::Edit > m_editExecutable;
	Ref< ui::EditList > m_listKeys;
	Ref< ui::EditList > m_listValues;

	Feature::Platform* m_selectedPlatform;
	std::wstring m_selectedKey;

	void selectPlatform(Feature::Platform* platform);

	void selectKey(const std::wstring& key);
};

	}
}

