/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/ShaderModuleEditorPage.h"

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
#include "Editor/PropertiesView.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/ShaderModule.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/Panel.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageGlsl.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"

namespace traktor::render
{
namespace
{

struct DependencyCharacter : public RefCountImpl< ui::RichEdit::ISpecialCharacter >
{
	editor::IEditor* editor;
	Guid id;
	std::wstring path;

	explicit DependencyCharacter(editor::IEditor* editor_, const Guid& id_, const std::wstring& path_)
		: editor(editor_)
		, id(id_)
		, path(path_)
	{
	}

	virtual int32_t measureWidth(const ui::RichEdit* richEdit) const override final
	{
		return richEdit->getFontMetric().getExtent(path).cx;
	}

	virtual void draw(ui::Canvas& canvas, const ui::Rect& rc) const override final
	{
		const int32_t h = 1;
		canvas.setBackground(canvas.getForeground());
		canvas.fillRect(ui::Rect(rc.left, rc.bottom - h, rc.right, rc.bottom));
		canvas.drawText(rc, path, ui::AnCenter, ui::AnCenter);
	}

	virtual void mouseButtonDown(ui::MouseButtonDownEvent* event) const override final
	{
	}

	virtual void mouseButtonUp(ui::MouseButtonUpEvent* event) const override final
	{
	}

	virtual void mouseDoubleClick(ui::MouseDoubleClickEvent* event) const override final
	{
		Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(id);
		if (instance)
			editor->openEditor(instance);
	}
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ShaderModuleEditorPage", ShaderModuleEditorPage, editor::IEditorPage)

ShaderModuleEditorPage::ShaderModuleEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
	: m_editor(editor)
	, m_site(site)
	, m_document(document)
{
}

bool ShaderModuleEditorPage::create(ui::Container* parent)
{
	m_module = m_document->getObject< ShaderModule >(0);
	if (!m_module)
		return false;

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangingEvent >(this, &ShaderModuleEditorPage::eventPropertiesChanging);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	m_edit = new ui::SyntaxRichEdit();
	if (!m_edit->create(parent, L"", ui::WsDoubleBuffer | ui::SyntaxRichEdit::WsNoClipboard))
		return false;
	m_edit->setLanguage(new ui::SyntaxLanguageGlsl());
	m_edit->addEventHandler< ui::ContentChangeEvent >(this, &ShaderModuleEditorPage::eventScriptChange);
	m_edit->addEventHandler< ui::MouseButtonUpEvent >(this, &ShaderModuleEditorPage::eventScriptButtonUp);

	const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
	const int32_t fontSize = m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11);
	m_edit->setFont(ui::Font(font, ui::Unit(fontSize)));

	if (m_module)
	{
		// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
		m_edit->setText(m_module->escape([&](const Guid& g) -> std::wstring {
			const db::Instance* instance = m_editor->getSourceDatabase()->getInstance(g);
			if (instance)
			{
				const wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(m_editor, g, instance->getPath()));
				return std::wstring(1, ch);
			}
			else
				return L"\"\"";
		}));
	}

	m_editMenu = new ui::Menu();
	m_editMenu->add(new ui::MenuItem(ui::Command(L"Script.Editor.AddUsingStatement"), i18n::Text(L"SCRIPT_EDITOR_ADD_USING")));

	m_propertiesView->setPropertyObject(m_module);

	parent->addEventHandler< ui::TimerEvent >(this, &ShaderModuleEditorPage::eventTimer);
	parent->startTimer(100);

	return true;
}

void ShaderModuleEditorPage::destroy()
{
	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);

	safeDestroy(m_edit);
	safeDestroy(m_propertiesView);

	m_site = nullptr;
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

		const std::wstring selectedText = m_edit->getSelectedText(
			[&](wchar_t ch) -> std::wstring {
				return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
			},
			[&](const ui::RichEdit::ISpecialCharacter* sc) -> std::wstring {
				const DependencyCharacter* dc = static_cast< const DependencyCharacter* >(sc);
				return L"\\" + dc->id.format();
			});
		clipboard->setText(selectedText);
	}
	else if (command == L"Editor.Cut")
	{
		ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
		if (!clipboard)
			return false;

		if (!m_edit->hasFocus())
			return false;

		m_document->push(new PropertyInteger(m_edit->getCaretOffset()));

		const std::wstring selectedText = m_edit->getSelectedText(
			[&](wchar_t ch) -> std::wstring {
				return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
			},
			[&](const ui::RichEdit::ISpecialCharacter* sc) -> std::wstring {
				const DependencyCharacter* dc = static_cast< const DependencyCharacter* >(sc);
				return L"\\" + dc->id.format();
			});
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

		m_document->push(new PropertyInteger(m_edit->getCaretOffset()));

		m_edit->deleteSelection();

		ShaderModule pasteModule(clipboard->getText());
		m_edit->insert(pasteModule.escape([&](const Guid& g) -> std::wstring {
			const db::Instance* instance = m_editor->getSourceDatabase()->getInstance(g);
			if (instance)
			{
				wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(m_editor, g, instance->getPath()));
				return std::wstring(1, ch);
			}
			else
				return L"\"\"";
		}));
	}
	else if (command == L"Editor.Undo")
	{
		Ref< const PropertyInteger > meta;
		if (
			m_module &&
			m_document->undo(new PropertyInteger(m_edit->getCaretOffset()), meta))
		{
			m_module = m_document->getObject< ShaderModule >(0);

			// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
			m_edit->setText(m_module->escape([&](const Guid& g) -> std::wstring {
				const db::Instance* instance = m_editor->getSourceDatabase()->getInstance(g);
				if (instance)
				{
					wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(m_editor, g, instance->getPath()));
					return std::wstring(1, ch);
				}
				else
					return L"\"\"";
			}));
			m_edit->placeCaret(*meta, true);

			m_propertiesView->setPropertyObject(m_module);
		}
	}
	else if (command == L"Editor.Redo")
	{
		Ref< const PropertyInteger > meta;
		if (
			m_module &&
			m_document->redo(meta))
		{
			m_module = m_document->getObject< ShaderModule >(0);

			// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
			m_edit->setText(m_module->escape([&](const Guid& g) -> std::wstring {
				const db::Instance* instance = m_editor->getSourceDatabase()->getInstance(g);
				if (instance)
				{
					wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(m_editor, g, instance->getPath()));
					return std::wstring(1, ch);
				}
				else
					return L"\"\"";
			}));
			m_edit->placeCaret(*meta, true);

			m_propertiesView->setPropertyObject(m_module);
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

void ShaderModuleEditorPage::eventPropertiesChanging(ui::ContentChangingEvent* event)
{
	m_document->push(new PropertyInteger(m_edit->getCaretOffset()));
}

void ShaderModuleEditorPage::eventScriptChange(ui::ContentChangeEvent* event)
{
	m_compileCountDown = 3;
}

void ShaderModuleEditorPage::eventScriptButtonUp(ui::MouseButtonUpEvent* event)
{
	if (!m_module)
		return;

	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* menuItem = m_editMenu->showModal(m_edit, event->getPosition());
	if (menuItem)
	{
		if (menuItem->getCommand() == L"Script.Editor.AddUsingStatement")
		{
			Ref< db::Instance > instance = m_editor->browseInstance(type_of< ShaderModule >());
			if (instance)
			{
				m_document->push(new PropertyInteger(m_edit->getCaretOffset()));

				int32_t offset = m_edit->getCaretOffset();
				int32_t line = m_edit->getLineFromOffset(offset);

				offset = m_edit->getLineOffset(line);

				const wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(m_editor, instance->getGuid(), instance->getPath()));
				m_edit->placeCaret(offset, true);
				m_edit->insert(L"#using " + std::wstring(1, ch) + L"\n");
			}
		}
	}

	event->consume();
}

void ShaderModuleEditorPage::eventTimer(ui::TimerEvent* event)
{
	if (--m_compileCountDown == 0)
	{
		if (!m_module)
			return;

		m_document->push(new PropertyInteger(m_edit->getCaretOffset()));

		// Transform editor text into "escaped" text.
		std::wstring text = m_edit->getText(
			[&](wchar_t ch) -> std::wstring {
				return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
			},
			[&](const ui::RichEdit::ISpecialCharacter* sc) -> std::wstring {
				const DependencyCharacter* dc = static_cast< const DependencyCharacter* >(sc);
				return L"\\" + dc->id.format();
			});

		// Update module with text.
		m_module->setTextDirect(text);
	}
}

}
