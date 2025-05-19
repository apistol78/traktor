/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"

namespace traktor::editor
{

class IDocument;
class IEditor;

}

namespace traktor::ui
{

class Container;
class SizeEvent;
class StatusBar;

}

namespace traktor::uikit
{

class PreviewControl;

class PreviewEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit PreviewEditor(editor::IEditor* editor, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IDocument* m_document;
	Ref< PreviewControl > m_previewControl;
	Ref< ui::Container > m_container;
	Ref< ui::StatusBar > m_statusBar;

	void eventPreviewSize(ui::SizeEvent* event);
};

}
