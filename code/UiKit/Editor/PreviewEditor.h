/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IObjectEditor.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace script
	{

class IScriptContext;

	}

	namespace ui
	{
	
class Container;
class SizeEvent;
class StatusBar;
	
	}

	namespace uikit
	{

class PreviewControl;

class PreviewEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	explicit PreviewEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< script::IScriptContext > m_scriptContext;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< PreviewControl > m_previewControl;
	Ref< ui::Container > m_container;
	Ref< ui::StatusBar > m_statusBar;

	void eventPreviewSize(ui::SizeEvent* event);
};

	}
}

