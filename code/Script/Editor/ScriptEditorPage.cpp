/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/Preprocessor.h"
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
#include "Script/IScriptCompiler.h"
#include "Script/IScriptProfiler.h"
#include "Script/StackFrame.h"
#include "Script/Editor/IScriptOutline.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptClassesView.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Script/Editor/ScriptEditorPage.h"
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
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/Panel.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageLua.h"
#include "Ui/StatusBar/StatusBar.h"

namespace traktor::script
{
	namespace
	{

struct DependencyCharacter : public RefCountImpl< ui::RichEdit::ISpecialCharacter >
{
	editor::IEditor* editor;
	Guid id;
	std::wstring path;

	explicit DependencyCharacter(editor::IEditor* editor_, const Guid& id_, const std::wstring& path_)
	:	editor(editor_)
	,	id(id_)
	,	path(path_)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptEditorPage", ScriptEditorPage, editor::IEditorPage)

ScriptEditorPage::ScriptEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_compileCountDown(0)
,	m_debugBreadcrumbAttribute(0)
,	m_debugCurrentAttribute(0)
{
	m_bitmapFunction = new ui::StyleBitmap(L"Script.DefineGlobalFunction");
	m_bitmapFunctionLocal = new ui::StyleBitmap(L"Script.DefineLocalFunction");
	m_bitmapFunctionReference = new ui::StyleBitmap(L"Script.ReferenceFunction");
	m_assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
}

bool ScriptEditorPage::create(ui::Container* parent)
{
	m_script = m_document->getObject< Script >(0);
	if (!m_script)
		return false;

	// Explorer panel container.
	m_containerExplorer = new ui::Container();
	m_containerExplorer->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut));
	m_containerExplorer->setText(i18n::Text(L"SCRIPT_EDITOR_EXPLORER"));

	Ref< ui::Tab > tab = new ui::Tab();
	if (!tab->create(m_containerExplorer, ui::WsNone))
		return false;

	Ref< ui::TabPage > tabOutline = new ui::TabPage();
	if (!tabOutline->create(tab, i18n::Text(L"SCRIPT_EDITOR_OUTLINE"), new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut)))
		return false;

	m_outlineGrid = new ui::GridView();
	if (!m_outlineGrid->create(tabOutline, ui::GridView::WsColumnHeader |ui::WsDoubleBuffer))
		return false;
	m_outlineGrid->addColumn(new ui::GridColumn(L"", 30_ut));
	m_outlineGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_NAME"), 165_ut));
	m_outlineGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_LINE"), 45_ut));
	m_outlineGrid->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptEditorPage::eventOutlineDoubleClick);

	Ref< ui::TabPage > tabClasses = new ui::TabPage();
	if (!tabClasses->create(tab, i18n::Text(L"SCRIPT_EDITOR_CLASSES"), new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut)))
		return false;

	m_classesView = new ScriptClassesView();
	if (!m_classesView->create(tabClasses))
		return false;

	tab->addPage(tabOutline);
	tab->addPage(tabClasses);
	tab->setActivePage(tabOutline);

	m_site->createAdditionalPanel(m_containerExplorer, 300_ut, false);

	// Edit area panel.
	Ref< ui::Container > containerEdit = new ui::Container();
	if (!containerEdit->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0_ut, 0_ut)))
		return false;

	m_edit = new ui::SyntaxRichEdit();
	if (!m_edit->create(containerEdit, L"", ui::WsDoubleBuffer | ui::SyntaxRichEdit::WsNoClipboard))
		return false;
	m_edit->addImage(new ui::StyleBitmap(L"Script.Breakpoint"), 1);
	m_edit->addEventHandler< ui::ContentChangeEvent >(this, &ScriptEditorPage::eventScriptChange);
	m_edit->addEventHandler< ui::MouseButtonDownEvent >(this, &ScriptEditorPage::eventScriptButtonDown);
	m_edit->addEventHandler< ui::MouseButtonUpEvent >(this, &ScriptEditorPage::eventScriptButtonUp);

	const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
	const int32_t fontSize = m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11);
	m_edit->setFont(ui::Font(font, ui::Unit(fontSize)));

	if (m_script)
	{
		// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
		m_edit->setText(m_script->escape([&] (const Guid& g) -> std::wstring {
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

	const ui::StyleSheet* ss = ui::Application::getInstance()->getStyleSheet();
	m_debugBreadcrumbAttribute = m_edit->addBackgroundAttribute(ss->getColor(this, L"background-debug-breadcrumb-line"));
	m_debugCurrentAttribute = m_edit->addBackgroundAttribute(ss->getColor(this, L"background-debug-current-line"));

	m_editMenu = new ui::Menu();
	m_editMenu->add(new ui::MenuItem(ui::Command(L"Script.Editor.AddUsingStatement"), i18n::Text(L"SCRIPT_EDITOR_ADD_USING")));

	m_compileStatus = new ui::StatusBar();
	if (!m_compileStatus->create(containerEdit))
		return false;
	m_compileStatus->addColumn(-1);

	// Create language specific implementations.
	{
		const std::wstring syntaxLanguageTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.SyntaxLanguageType");
		const TypeInfo* syntaxLanguageType = TypeInfo::find(syntaxLanguageTypeName.c_str());
		if (syntaxLanguageType)
		{
			Ref< ui::SyntaxLanguage > syntaxLanguage = dynamic_type_cast< ui::SyntaxLanguage* >(syntaxLanguageType->createInstance());
			T_ASSERT(syntaxLanguage);
			m_edit->setLanguage(syntaxLanguage);
		}

		const std::wstring scriptCompilerTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.ScriptCompilerType");
		const TypeInfo* scriptCompilerType = TypeInfo::find(scriptCompilerTypeName.c_str());
		if (scriptCompilerType)
		{
			m_scriptCompiler = dynamic_type_cast< IScriptCompiler* >(scriptCompilerType->createInstance());
			T_ASSERT(m_scriptCompiler);
		}

		const std::wstring scriptOutlineTypeName = L"traktor.script.ScriptOutlineLua";
		const TypeInfo* scriptOutlineType = TypeInfo::find(scriptOutlineTypeName.c_str());
		if (scriptOutlineType)
		{
			m_scriptOutline = dynamic_type_cast< IScriptOutline* >(scriptOutlineType->createInstance());
			T_ASSERT( m_scriptOutline);
		}
	}

	// Setup compile timer.
	if (m_script && m_scriptCompiler)
	{
		parent->addEventHandler< ui::TimerEvent >(this, &ScriptEditorPage::eventTimer);
		parent->startTimer(100);
		m_compileCountDown = 1;
	}

	// Create preprocessor.
	m_preprocessor = new Preprocessor();

	// Get debugger implementation.
	m_scriptDebuggerSessions = m_editor->getObjectStore()->get< IScriptDebuggerSessions >();
	if (m_scriptDebuggerSessions)
	{
		m_scriptDebuggerSessions->addListener(this);

		// Get all breakpoints.
		const Guid instanceGuid = m_document->getInstance(0)->getGuid();
		for (int32_t i = 0; i < m_edit->getLineCount(); ++i)
		{
			if (m_scriptDebuggerSessions->haveBreakpoint(instanceGuid, i))
			{
				m_edit->setLineData(i, new PropertyBoolean(true));
				m_edit->setImage(i, 0);
			}
		}
	}

	return true;
}

void ScriptEditorPage::destroy()
{
	if (m_scriptDebuggerSessions)
	{
		m_scriptDebuggerSessions->removeListener(this);
		m_scriptDebuggerSessions = nullptr;
	}

	m_preprocessor = nullptr;
	m_scriptCompiler = nullptr;

	// Destroy panels.
	if (m_containerExplorer)
		m_site->destroyAdditionalPanel(m_containerExplorer);
	
	safeDestroy(m_containerExplorer);
	m_site = nullptr;
}

bool ScriptEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const int32_t dropOffset = m_edit->getOffsetFromPosition(m_edit->screenToClient(position));
	if (dropOffset < 0)
		return false;

	const wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(m_editor, instance->getGuid(), instance->getPath()));
	m_edit->placeCaret(dropOffset, true);
	m_edit->insert(std::wstring(1, ch));
	return true;
}

bool ScriptEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Copy")
	{
		ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
		if (!clipboard)
			return false;

		if (!m_edit->hasFocus())
			return false;

		const std::wstring selectedText = m_edit->getSelectedText(
			[&] (wchar_t ch) -> std::wstring {
				return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
			},
			[&] (const ui::RichEdit::ISpecialCharacter* sc) -> std::wstring {
				const DependencyCharacter* dc = static_cast< const DependencyCharacter* >(sc);
				return L"\\" + dc->id.format();
			}
		);
		clipboard->setText(selectedText);
	}
	else if (command == L"Editor.Cut")
	{
		ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
		if (!clipboard)
			return false;

		if (!m_edit->hasFocus())
			return false;

		const std::wstring selectedText = m_edit->getSelectedText(
			[&] (wchar_t ch) -> std::wstring {
				return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
			},
			[&] (const ui::RichEdit::ISpecialCharacter* sc) -> std::wstring {
				const DependencyCharacter* dc = static_cast< const DependencyCharacter* >(sc);
				return L"\\" + dc->id.format();
			}
		);
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

		Script pasteScript(clipboard->getText());
		m_edit->insert(pasteScript.escape([&] (const Guid& g) -> std::wstring {
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
			m_script &&
			m_document->undo(new PropertyInteger(m_edit->getCaretOffset()), meta)
		)
		{
			m_script = m_document->getObject< Script >(0);

			// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
			m_edit->setText(m_script->escape([&] (const Guid& g) -> std::wstring {
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

			updateBreakpoints();
		}
	}
	else if (command == L"Editor.Redo")
	{
		Ref< const PropertyInteger > meta;
		if (
			m_script &&
			m_document->redo(meta)
		)
		{
			m_script = m_document->getObject< Script >(0);

			// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
			m_edit->setText(m_script->escape([&] (const Guid& g) -> std::wstring {
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

			updateBreakpoints();
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

void ScriptEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void ScriptEditorPage::syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message)
{
	m_compileStatus->setText(0, i18n::Format(L"SCRIPT_EDITOR_STATUS_SYNTAX_ERROR", int32_t(line), message));
	m_compileStatus->setAlert(true);
	if (line > 0)
		m_edit->setErrorHighlight(line - 1);
}

void ScriptEditorPage::otherError(const std::wstring& message)
{
	m_compileStatus->setText(0, i18n::Format(L"SCRIPT_EDITOR_STATUS_OTHER_ERROR", message));
	m_compileStatus->setAlert(true);
}

void ScriptEditorPage::debugeeStateChange(IScriptDebugger* scriptDebugger)
{
	for (int32_t line = 0; line < m_edit->getLineCount(); ++line)
		m_edit->setBackgroundAttribute(line, 0xffff);

	if (!scriptDebugger->isRunning())
	{
		const Guid instanceGuid = m_document->getInstance(0)->getGuid();

		Ref< StackFrame > sf;
		for (uint32_t depth = 0; scriptDebugger->captureStackFrame(depth, sf); ++depth)
		{
			T_FATAL_ASSERT (sf);
			if (sf->getScriptId() == instanceGuid)
			{
				// Set breadcrumb trail of execution.
				AlignedVector< uint32_t > breadcrumbs;
				if (scriptDebugger->captureBreadcrumbs(breadcrumbs))
				{
					for (auto line : breadcrumbs)
						m_edit->setBackgroundAttribute(line, m_debugBreadcrumbAttribute);
				}

				// Highlight current, breaked, line.
				const int32_t line = (int32_t)sf->getLine();
				m_edit->setBackgroundAttribute(line, m_debugCurrentAttribute);
				break;
			}
		}
	}
}

void ScriptEditorPage::notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	scriptDebugger->addListener(this);
}

void ScriptEditorPage::notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	scriptDebugger->removeListener(this);
}

void ScriptEditorPage::notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

void ScriptEditorPage::notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

void ScriptEditorPage::updateBreakpoints()
{
	const Guid instanceGuid = m_document->getInstance(0)->getGuid();
	m_scriptDebuggerSessions->removeAllBreakpoints(instanceGuid);
	for (int32_t i = 0; i < m_edit->getLineCount(); ++i)
	{
		PropertyBoolean* p = dynamic_type_cast< PropertyBoolean* >(m_edit->getLineData(i));
		if (p)
		{
			m_scriptDebuggerSessions->setBreakpoint(instanceGuid, i);
			m_edit->setImage(i, 0);
		}
		else
			m_edit->setImage(i, 1);
	}
	m_edit->update();
}

void ScriptEditorPage::buildOutlineGrid(ui::GridView* grid, ui::GridRow* parent, const IScriptOutline::Node* on)
{
	while (on)
	{
		if (const IScriptOutline::FunctionNode* fn = dynamic_type_cast< const IScriptOutline::FunctionNode* >(on))
		{
			Ref< ui::GridRow > row = new ui::GridRow(0);

			row->add(fn->isLocal() ? m_bitmapFunctionLocal : m_bitmapFunction);
			row->add(fn->getName());
			row->add(toString(fn->getLine() + 1));

			buildOutlineGrid(grid, row, fn->getBody());

			if (parent)
				parent->addChild(row);
			else
				grid->addRow(row);
		}
		else if (const IScriptOutline::FunctionReferenceNode* frn = dynamic_type_cast< const IScriptOutline::FunctionReferenceNode* >(on))
		{
			Ref< ui::GridRow > row = new ui::GridRow(0);

			row->add(m_bitmapFunctionReference);
			row->add(frn->getName());
			row->add(toString(frn->getLine() + 1));

			if (parent)
				parent->addChild(row);
			else
				grid->addRow(row);
		}

		on = on->getNext();
	}
}

void ScriptEditorPage::eventOutlineDoubleClick(ui::MouseDoubleClickEvent* event)
{
	const ui::GridRow* selectedRow = m_outlineGrid->getSelectedRow();
	if (!selectedRow)
		return;

	const ui::GridItem* lineItem = selectedRow->get(2);
	const int32_t line = parseString< int32_t >(lineItem->getText()) - 1;
	if (line >= 0)
		m_edit->placeCaret(m_edit->getLineOffset(line), true);
}

void ScriptEditorPage::eventScriptChange(ui::ContentChangeEvent* event)
{
	if (!m_script)
		return;

	// Transform editor text into "escaped" text.
	const std::wstring text = m_edit->getText(
		[&] (wchar_t ch) -> std::wstring {
			return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
		},
		[&] (const ui::RichEdit::ISpecialCharacter* sc) -> std::wstring {
			const DependencyCharacter* dc = static_cast< const DependencyCharacter* >(sc);
			return L"\\" + dc->id.format();
		}
	);

	// Update script with text.
	m_script->setTextDirect(text);

	m_compileCountDown = 3;
	m_compileStatus->setText(0, i18n::Text(L"SCRIPT_EDITOR_STATUS_READY"));
	m_compileStatus->setAlert(false);

	updateBreakpoints();
}

void ScriptEditorPage::eventScriptButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getPosition().x >= m_edit->getMarginWidth() || event->getButton() != ui::MbtLeft)
		return;

	const int32_t line = m_edit->getLineFromPosition(event->getPosition().y);
	if (line >= 0)
	{
		if (!m_edit->getLineData(line))
			m_edit->setLineData(line, new PropertyBoolean(true));
		else
			m_edit->setLineData(line, 0);

		updateBreakpoints();
	}

	event->consume();
}

void ScriptEditorPage::eventScriptButtonUp(ui::MouseButtonUpEvent* event)
{
	if (!m_script)
		return;

	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* menuItem = m_editMenu->showModal(m_edit, event->getPosition());
	if (menuItem)
	{
		if (menuItem->getCommand() == L"Script.Editor.AddUsingStatement")
		{
			Ref< db::Instance > instance = m_editor->browseInstance(type_of< Script >());
			if (instance)
			{
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

void ScriptEditorPage::eventTimer(ui::TimerEvent* event)
{
	T_FATAL_ASSERT (m_script);

	if (--m_compileCountDown == 0)
	{
		// This is triggered by script change; push for undo here
		// as we don't want to keep pushing for all input.
		if (m_document)
			m_document->push(new PropertyInteger(m_edit->getCaretOffset()));

		if (m_scriptCompiler)
		{
			// Take snapshot of script and try to compile it.
			std::wstring script;
			std::set< std::wstring > usings;
			m_preprocessor->evaluate(m_edit->getText(), script, usings);

			if (m_scriptCompiler->compile(L"", script, this))
			{
				// Reset error status.
				m_compileStatus->setText(0, i18n::Text(L"SCRIPT_EDITOR_STATUS_READY"));
				m_compileStatus->setAlert(false);
				m_edit->setErrorHighlight(-1);

				if (m_scriptOutline)
				{
					m_outlineGrid->removeAllRows();

					Ref< IScriptOutline::Node > on = m_scriptOutline->parse(script);
					buildOutlineGrid(m_outlineGrid, 0, on);
				}
			}
		}
	}
}

}
