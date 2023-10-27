/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/ShaderModule.h"
#include "Render/Editor/Shader/ShaderModuleEditorPage.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"
#include "Ui/Splitter.h"
#include "Ui/Panel.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageGlsl.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ShaderModuleEditorPage", ShaderModuleEditorPage, editor::IEditorPage)

ShaderModuleEditorPage::ShaderModuleEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool ShaderModuleEditorPage::create(ui::Container* parent)
{
	m_module = m_document->getObject< ShaderModule >(0);
	if (!m_module)
		return false;

	m_edit = new ui::SyntaxRichEdit();
	if (!m_edit->create(parent, L"", ui::WsDoubleBuffer | ui::SyntaxRichEdit::WsNoClipboard))
		return false;
	m_edit->setLanguage(new ui::SyntaxLanguageGlsl());
	m_edit->addImage(new ui::StyleBitmap(L"Script.Breakpoint"), 1);
	m_edit->addEventHandler< ui::ContentChangeEvent >(this, &ShaderModuleEditorPage::eventScriptChange);

	const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
	const int32_t fontSize = m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11);
	m_edit->setFont(ui::Font(font, ui::Unit(fontSize)));

	if (m_module)
		m_edit->setText(m_module->getText());

	return true;
}

void ShaderModuleEditorPage::destroy()
{
}

bool ShaderModuleEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool ShaderModuleEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Copy")
	{
		ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
		if (!clipboard)
			return false;

		if (!m_edit->hasFocus())
			return false;

		const std::wstring selectedText = m_edit->getSelectedText();
		clipboard->setText(selectedText);
	}
	else if (command == L"Editor.Cut")
	{
		ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
		if (!clipboard)
			return false;

		if (!m_edit->hasFocus())
			return false;

		const std::wstring selectedText = m_edit->getSelectedText();
		clipboard->setText(selectedText);

		m_edit->deleteSelection();
	}
	else if (command == L"Editor.Paste")
	{
		ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
		if (!clipboard)
			return false;

		if (!m_edit->hasFocus())
			return false;

		m_edit->deleteSelection();
		m_edit->insert(clipboard->getText());
	}
	else if (command == L"Editor.Undo")
	{
		Ref< const PropertyInteger > meta;
		if (
			m_module &&
			m_document->undo(new PropertyInteger(m_edit->getCaretOffset()), meta)
		)
		{
			m_module = m_document->getObject< ShaderModule >(0);
			m_edit->setText(m_module->getText());
			m_edit->placeCaret(*meta, true);
		}
	}
	else if (command == L"Editor.Redo")
	{
		Ref< const PropertyInteger > meta;
		if (
			m_module &&
			m_document->redo(meta)
		)
		{
			m_module = m_document->getObject< ShaderModule >(0);
			m_edit->setText(m_module->getText());
			m_edit->placeCaret(*meta, true);
		}
	}
	else if (command == L"Editor.Find")
	{
		m_edit->find();
	}
	else if (command == L"Editor.FindNext")
	{
		m_edit->findNext();
	}
	else if (command == L"Editor.Replace")
	{
		m_edit->replace();
	}
	else if (command == L"Editor.ReplaceAll")
	{
		m_edit->replaceAll();
	}
	else if (command == L"Editor.SelectAll")
	{
		m_edit->selectAll();
	}
	else if (command == L"Editor.Unselect")
	{
		m_edit->unselect();
	}
	else if (command == L"Editor.SettingsChanged")
	{
		const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
		const int32_t fontSize = m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11);
		m_edit->setFont(ui::Font(font, ui::Unit(fontSize)));
		m_edit->update();
	}
	else if (command == L"Script.Editor.GotoLine")
	{
		const int32_t lineOffset = m_edit->getLineOffset(command.getId());
		m_edit->placeCaret(lineOffset, true);
	}
	else
		return false;

	return true;
}

void ShaderModuleEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void ShaderModuleEditorPage::eventScriptChange(ui::ContentChangeEvent* event)
{
	if (!m_module)
		return;

	const std::wstring& text = m_edit->getText();
	m_module->setText(text);
}

}
