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
#include "Script/IScriptProfiler.h"
#include "Script/StackFrame.h"
#include "Script/Editor/IScriptOutline.h"
#include "Script/Editor/Preprocessor.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptAsset.h"
#include "Script/Editor/ScriptClassesView.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Script/Editor/ScriptEditorPage.h"
#include "Script/Editor/SearchControl.h"
#include "Script/Editor/SearchEvent.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/Panel.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageLua.h"
#include "Ui/Custom/StatusBar/StatusBar.h"

namespace traktor
{
	namespace script
	{
		namespace
		{
		
struct DependencyCharacter : public RefCountImpl< ui::custom::RichEdit::ISpecialCharacter >
{
	Guid id;
	std::wstring path;

	DependencyCharacter(const Guid& id_, const std::wstring& path_)
	:	id(id_)
	,	path(path_)
	{
	}

	virtual int32_t measureWidth(const ui::custom::RichEdit* richEdit) const T_OVERRIDE T_FINAL
	{
		return richEdit->getTextExtent(path).cx;
	}

	virtual void draw(ui::Canvas& canvas, const ui::Rect& rc) const T_OVERRIDE T_FINAL
	{
		canvas.drawText(rc, path, ui::AnCenter, ui::AnCenter);
	}

	virtual void mouseButtonDown(ui::MouseButtonDownEvent* event) const T_OVERRIDE T_FINAL
	{
	}

	virtual void mouseButtonUp(ui::MouseButtonUpEvent* event) const T_OVERRIDE T_FINAL
	{
	}
};
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptEditorPage", ScriptEditorPage, editor::IEditorPage)

ScriptEditorPage::ScriptEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_compileCountDown(0)
,	m_foundLineAttribute(0)
,	m_debugLineAttribute(0)
,	m_debugLineLast(-1)
{
	m_bitmapFunction = new ui::StyleBitmap(L"Script.DefineGlobalFunction");
	m_bitmapFunctionLocal = new ui::StyleBitmap(L"Script.DefineLocalFunction");
	m_bitmapFunctionReference = new ui::StyleBitmap(L"Script.ReferenceFunction");
	m_assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
}

bool ScriptEditorPage::create(ui::Container* parent)
{
	m_script = m_document->getObject< Script >(0);
	m_scriptAsset = m_document->getObject< ScriptAsset >(0);
	if (!m_script && !m_scriptAsset)
		return false;

	// Explorer panel container.
	m_containerExplorer = new ui::Container();
	m_containerExplorer->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));
	m_containerExplorer->setText(i18n::Text(L"SCRIPT_EDITOR_EXPLORER"));

	Ref< ui::Tab > tab = new ui::Tab();
	if (!tab->create(m_containerExplorer, ui::WsNone))
		return false;

	Ref< ui::TabPage > tabOutline = new ui::TabPage();
	if (!tabOutline->create(tab, i18n::Text(L"SCRIPT_EDITOR_OUTLINE"), new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_outlineGrid = new ui::custom::GridView();
	if (!m_outlineGrid->create(tabOutline, ui::custom::GridView::WsColumnHeader |ui::WsDoubleBuffer))
		return false;
	m_outlineGrid->addColumn(new ui::custom::GridColumn(L"", ui::scaleBySystemDPI(30)));
	m_outlineGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_NAME"), ui::scaleBySystemDPI(165)));
	m_outlineGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_LINE"), ui::scaleBySystemDPI(45)));
	m_outlineGrid->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptEditorPage::eventOutlineDoubleClick);

	Ref< ui::TabPage > tabClasses = new ui::TabPage();
	if (!tabClasses->create(tab, i18n::Text(L"SCRIPT_EDITOR_CLASSES"), new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_classesView = new ScriptClassesView();
	if (!m_classesView->create(tabClasses))
		return false;

	tab->addPage(tabOutline);
	tab->addPage(tabClasses);
	tab->setActivePage(tabOutline);

	m_site->createAdditionalPanel(m_containerExplorer, ui::scaleBySystemDPI(300), false);

	// Edit area panel.
	Ref< ui::Container > containerEdit = new ui::Container();

	if (m_script)
	{
		if (!containerEdit->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0, 0)))
			return false;

		Ref< ui::custom::ToolBar > toolBarEdit = new ui::custom::ToolBar();
		toolBarEdit->create(containerEdit);
		toolBarEdit->addImage(new ui::StyleBitmap(L"Script.RemoveBreakpoints"), 1);
		toolBarEdit->addImage(new ui::StyleBitmap(L"Script.ToggleComments"), 1);
		toolBarEdit->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_TOGGLE_COMMENTS"), 1, ui::Command(L"Script.Editor.ToggleComments")));
		toolBarEdit->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_REMOVE_ALL_BREAKPOINTS"), 0, ui::Command(L"Script.Editor.RemoveAllBreakpoints")));
		toolBarEdit->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ScriptEditorPage::eventToolBarEditClick);
	}
	else if (m_scriptAsset)
	{
		if (!containerEdit->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0)))
			return false;
	}

	m_edit = new ui::custom::SyntaxRichEdit();
	if (!m_edit->create(containerEdit, L"", ui::WsDoubleBuffer))
		return false;
	m_edit->addImage(new ui::StyleBitmap(L"Script.Breakpoint"), 1);

	if (m_script)
	{
		// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
		m_edit->setText(m_script->escape([&] (const Guid& g) -> std::wstring {
			const db::Instance* instance = m_editor->getSourceDatabase()->getInstance(g);
			if (instance)
			{
				wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(g, instance->getPath()));
				return std::wstring(1, ch);
			}
			else
				return L"\"\"";
		}));
	}
	else if (m_scriptAsset)
	{
		Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + Path(m_scriptAsset->getFileName().getOriginal()));
		Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
		if (file)
		{
			StringOutputStream ss;
			std::wstring line;

			// Read script using utf-8 encoding.
			Utf8Encoding encoding;
			StringReader sr(file, &encoding);
			while (sr.readLine(line) >= 0)
				ss << line << Endl;

			m_edit->setText(ss.str());
		}
		else
			log::error << L"Unable to open external script (" << m_scriptAsset->getFileName().getOriginal() << L")" << Endl;
	}

	std::wstring font = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.Font", L"Consolas");
	int32_t fontSize = m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.FontSize", 14);
	m_edit->setFont(ui::Font(font, fontSize));

	m_edit->addEventHandler< ui::ContentChangeEvent >(this, &ScriptEditorPage::eventScriptChange);
	m_edit->addEventHandler< ui::MouseButtonDownEvent >(this, &ScriptEditorPage::eventScriptButtonDown);
	m_edit->addEventHandler< ui::MouseButtonUpEvent >(this, &ScriptEditorPage::eventScriptButtonUp);
	m_edit->addEventHandler< ui::SizeEvent >(this, &ScriptEditorPage::eventScriptSize);

	const ui::StyleSheet* ss = ui::Application::getInstance()->getStyleSheet();
	m_foundLineAttribute = m_edit->addBackgroundAttribute(ss->getColor(this, L"background-found-line"));
	m_debugLineAttribute = m_edit->addBackgroundAttribute(ss->getColor(this, L"background-debug-line"));

	m_editMenu = new ui::PopupMenu();
	m_editMenu->create();
	m_editMenu->add(new ui::MenuItem(ui::Command(L"Script.Editor.AddUsingStatement"), i18n::Text(L"SCRIPT_EDITOR_ADD_USING")));

	m_searchControl = new SearchControl();
	m_searchControl->create(m_edit);
	m_searchControl->hide();
	m_searchControl->addEventHandler< SearchEvent >(this, &ScriptEditorPage::eventSearch);

	m_compileStatus = new ui::custom::StatusBar();
	if (!m_compileStatus->create(containerEdit))
		return false;

	// Create language specific implementations.
	{
		std::wstring syntaxLanguageTypeName = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.SyntaxLanguageType");
		const TypeInfo* syntaxLanguageType = TypeInfo::find(syntaxLanguageTypeName);
		if (syntaxLanguageType)
		{
			Ref< ui::custom::SyntaxLanguage > syntaxLanguage = dynamic_type_cast< ui::custom::SyntaxLanguage* >(syntaxLanguageType->createInstance());
			T_ASSERT (syntaxLanguage);
			m_edit->setLanguage(syntaxLanguage);
		}

		std::wstring scriptManagerTypeName = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.ScriptManagerType");
		const TypeInfo* scriptManagerType = TypeInfo::find(scriptManagerTypeName);
		if (scriptManagerType)
		{
			m_scriptManager = dynamic_type_cast< IScriptManager* >(scriptManagerType->createInstance());
			T_ASSERT (m_scriptManager);
		}

		std::wstring scriptOutlineTypeName = L"traktor.script.ScriptOutlineLua";
		const TypeInfo* scriptOutlineType = TypeInfo::find(scriptOutlineTypeName);
		if (scriptOutlineType)
		{
			m_scriptOutline = dynamic_type_cast< IScriptOutline* >(scriptOutlineType->createInstance());
			T_ASSERT( m_scriptOutline);
		}
	}

	// Setup compile timer.
	if (m_script && m_scriptManager)
	{
		parent->addEventHandler< ui::TimerEvent >(this, &ScriptEditorPage::eventTimer);
		parent->startTimer(100);
		m_compileCountDown = 1;
	}

	// Create preprocessor.
	m_preprocessor = new Preprocessor();

	// Get debugger implementation.
	m_scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (m_scriptDebuggerSessions)
	{
		m_scriptDebuggerSessions->addListener(this);

		// Get all breakpoints.
		Guid instanceGuid = m_document->getInstance(0)->getGuid();
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
	if (m_scriptDebuggerSessions != 0)
	{
		m_scriptDebuggerSessions->removeListener(this);
		m_scriptDebuggerSessions = 0;
	}

	m_preprocessor = 0;

	// Destroy panels.
	m_site->destroyAdditionalPanel(m_containerExplorer);

	safeDestroy(m_containerExplorer);
	safeDestroy(m_scriptManager);
}

void ScriptEditorPage::activate()
{
	m_edit->setFocus();
}

void ScriptEditorPage::deactivate()
{
}

bool ScriptEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	int32_t dropOffset = m_edit->getOffsetFromPosition(m_edit->screenToClient(position));
	if (dropOffset < 0)
		return false;

	wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(instance->getGuid(), instance->getPath()));

	m_edit->placeCaret(dropOffset);
	m_edit->insert(std::wstring(1, ch));

	return true;
}

bool ScriptEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Undo")
	{
		if (m_script && m_document->undo())
		{
			m_script = m_document->getObject< Script >(0);

			// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
			m_edit->setText(m_script->escape([&] (const Guid& g) -> std::wstring {
				const db::Instance* instance = m_editor->getSourceDatabase()->getInstance(g);
				if (instance)
				{
					wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(g, instance->getPath()));
					return std::wstring(1, ch);
				}
				else
					return L"\"\"";
			}));

			updateBreakpoints();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_script && m_document->redo())
		{
			m_script = m_document->getObject< Script >(0);

			// Escape text and set into editor, embedded dependencies are wrapped as "special characters".
			m_edit->setText(m_script->escape([&] (const Guid& g) -> std::wstring {
				const db::Instance* instance = m_editor->getSourceDatabase()->getInstance(g);
				if (instance)
				{
					wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(g, instance->getPath()));
					return std::wstring(1, ch);
				}
				else
					return L"\"\"";
			}));

			updateBreakpoints();
		}
	}
	else if (command == L"Editor.Find")
	{
		m_searchControl->show();
		m_searchControl->setFocus();
	}
	else if (command == L"Editor.FindNext")
	{
	}
	else if (command == L"Editor.Replace")
	{
	}
	else if (command == L"Editor.ReplaceAll")
	{
	}
	else if (command == L"Editor.SelectAll")
	{
		m_edit->selectAll();
	}
	else if (command == L"Editor.SettingsChanged")
	{
		std::wstring font = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.Font", L"Consolas");
		int32_t fontSize = m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.FontSize", 14);
		m_edit->setFont(ui::Font(font, fontSize));
		m_edit->update();
	}
	else if (command == L"Script.Editor.GotoLine")
	{
		int32_t lineOffset = m_edit->getLineOffset(command.getId());
		m_edit->placeCaret(lineOffset);
		m_edit->showLine(command.getId());
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
	m_compileStatus->setText(i18n::Format(L"SCRIPT_EDITOR_STATUS_SYNTAX_ERROR", int32_t(line), message));
	m_compileStatus->setAlert(true);
	if (line > 0)
		m_edit->setErrorHighlight(line - 1);
}

void ScriptEditorPage::otherError(const std::wstring& message)
{
	m_compileStatus->setText(i18n::Format(L"SCRIPT_EDITOR_STATUS_OTHER_ERROR", message));
	m_compileStatus->setAlert(true);
}

void ScriptEditorPage::debugeeStateChange(IScriptDebugger* scriptDebugger)
{
	if (m_debugLineLast >= 0)
	{
		m_edit->setBackgroundAttribute(m_debugLineLast, 0xffff);
		m_debugLineLast = -1;
	}
	if (!scriptDebugger->isRunning())
	{
		Guid instanceGuid = m_document->getInstance(0)->getGuid();
		Ref< StackFrame > sf;
		for (uint32_t depth = 0; scriptDebugger->captureStackFrame(depth, sf); ++depth)
		{
			T_FATAL_ASSERT (sf);
			if (sf->getScriptId() == instanceGuid)
			{
				int32_t line = int32_t(sf->getLine());
				m_edit->setBackgroundAttribute(line, m_debugLineAttribute);
				m_debugLineLast = line;
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
	Guid instanceGuid = m_document->getInstance(0)->getGuid();
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

void ScriptEditorPage::buildOutlineGrid(ui::custom::GridView* grid, ui::custom::GridRow* parent, const IScriptOutline::Node* on)
{
	while (on)
	{
		if (const IScriptOutline::FunctionNode* fn = dynamic_type_cast< const IScriptOutline::FunctionNode* >(on))
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);

			row->add(new ui::custom::GridItem(fn->isLocal() ? m_bitmapFunctionLocal : m_bitmapFunction));
			row->add(new ui::custom::GridItem(fn->getName()));
			row->add(new ui::custom::GridItem(toString(fn->getLine() + 1)));

			buildOutlineGrid(grid, row, fn->getBody());

			if (parent)
				parent->addChild(row);
			else
				grid->addRow(row);
		}
		else if (const IScriptOutline::FunctionReferenceNode* frn = dynamic_type_cast< const IScriptOutline::FunctionReferenceNode* >(on))
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);

			row->add(new ui::custom::GridItem(m_bitmapFunctionReference));
			row->add(new ui::custom::GridItem(frn->getName()));
			row->add(new ui::custom::GridItem(toString(frn->getLine() + 1)));

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
	const ui::custom::GridRow* selectedRow = m_outlineGrid->getSelectedRow();
	if (!selectedRow)
		return;

	const ui::custom::GridItem* lineItem = selectedRow->get(2);
	int32_t line = parseString< int32_t >(lineItem->getText()) - 1;
	if (line >= 0)
	{
		m_edit->showLine(line);
		m_edit->placeCaret(m_edit->getLineOffset(line));
	}
}

void ScriptEditorPage::eventToolBarEditClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"Script.Editor.ToggleComments")
	{
		//int32_t startOffset = m_edit->getSelectionStartOffset();
		//int32_t stopOffset = m_edit->getSelectionStopOffset();

		//if (startOffset < 0)
		//{
		//	int32_t caret = m_edit->getCaretOffset();
		//	int32_t caretLine = m_edit->getLineFromOffset(caret);
		//	startOffset = m_edit->getLineOffset(caretLine);
		//	stopOffset = startOffset + m_edit->getLineLength(caretLine);
		//}

		//if (startOffset >= 0 && stopOffset >= 0)
		//{
		//	std::wstring lineComment = m_edit->getLanguage()->lineComment();
		//	T_ASSERT (!lineComment.empty());

		//	int32_t startLine = m_edit->getLineFromOffset(startOffset);
		//	int32_t stopLine = m_edit->getLineFromOffset(stopOffset);

		//	for (int32_t i = startLine; i <= stopLine; ++i)
		//	{
		//		std::wstring line = m_edit->getLine(i);
		//		if (startsWith(line, lineComment))
		//			line = line.substr(2);
		//		else
		//			line = lineComment + line;
		//		m_edit->setLine(i, line);
		//	}

		//	m_script->setText(m_edit->getText());

		//	m_edit->updateLanguage(startLine, stopLine);
		//	m_edit->update();
		//}
	}
	else if (command == L"Script.Editor.RemoveAllBreakpoints")
	{
		int32_t lineCount = m_edit->getLineCount();
		for (int32_t i = 0; i < lineCount; ++i)
			m_edit->setLineData(i, 0);
		updateBreakpoints();
	}
}

void ScriptEditorPage::eventScriptChange(ui::ContentChangeEvent* event)
{
	if (!m_script)
		return;

	// Transform editor text into "escaped" text.
	std::wstring text = m_edit->getText(
		[&] (wchar_t ch) -> std::wstring {
			return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
		},
		[&] (const ui::custom::RichEdit::ISpecialCharacter* sc) -> std::wstring {
			const DependencyCharacter* dc = static_cast< const DependencyCharacter* >(sc);
			return L"\\" + dc->id.format();
		}
	);

	// Update script with text.
	m_script->setTextDirect(text);

	m_compileCountDown = 10;
	m_compileStatus->setText(i18n::Text(L"SCRIPT_EDITOR_STATUS_READY"));
	m_compileStatus->setAlert(false);

	updateBreakpoints();
}

void ScriptEditorPage::eventScriptButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getPosition().x >= m_edit->getMarginWidth() || event->getButton() != ui::MbtLeft)
		return;

	int32_t line = m_edit->getLineFromPosition(event->getPosition().y);
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

	Ref< ui::MenuItem > menuItem = m_editMenu->show(m_edit, event->getPosition());
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

				wchar_t ch = m_edit->addSpecialCharacter(new DependencyCharacter(instance->getGuid(), instance->getPath()));
				m_edit->placeCaret(offset);
				m_edit->insert(L"#using " + std::wstring(1, ch) + L"\n");
			}
		}
	}

	event->consume();
}

void ScriptEditorPage::eventScriptSize(ui::SizeEvent* event)
{
	ui::Size searchControlSize = m_searchControl->getPreferedSize();
	m_searchControl->setRect(ui::Rect(
		ui::Point(m_edit->getEditRect().getWidth() - searchControlSize.cx, 0),
		searchControlSize
	));
}

void ScriptEditorPage::eventSearch(SearchEvent* event)
{
	if (!event->isPreview())
	{
		int32_t caretLine = m_edit->getLineFromOffset(m_edit->getCaretOffset());
		int32_t line = caretLine;

		while (line < m_edit->getLineCount())
		{
			std::wstring text = m_edit->getLine(line);
			size_t p = text.find(event->getSearch());
			if (p != text.npos)
			{
				m_edit->showLine(line);
				m_edit->placeCaret(m_edit->getLineOffset(line) + int32_t(p));
				for (int32_t i = 0; i < m_edit->getLineCount(); ++i)
					m_edit->setBackgroundAttribute(i, (i == line) ? m_foundLineAttribute : 0xffff);
				break;
			}
			++line;
		}

		if (line >= m_edit->getLineCount())
		{
			line = 0;
			while (line < caretLine)
			{
				std::wstring text = m_edit->getLine(line);
				size_t p = text.find(event->getSearch());
				if (p != text.npos)
				{
					m_edit->showLine(line);
					m_edit->placeCaret(m_edit->getLineOffset(line) + int32_t(p));
					for (int32_t i = 0; i < m_edit->getLineCount(); ++i)
						m_edit->setBackgroundAttribute(i, (i == line) ? m_foundLineAttribute : 0xffff);
					break;
				}
				++line;
			}
		}
	}
	else
	{
		// See if any match exist in document, update search control hints.
		bool found = false;
		for (int32_t line = 0; line < m_edit->getLineCount(); ++line)
		{
			std::wstring text = m_edit->getLine(line);
			size_t p = text.find(event->getSearch());
			if (p != text.npos)
			{
				found = true;
				break;
			}
		}
		m_searchControl->setAnyMatchingHint(found);
	}
}

void ScriptEditorPage::eventTimer(ui::TimerEvent* event)
{
	T_FATAL_ASSERT (m_script);

	if (--m_compileCountDown == 0)
	{
		// This is triggered by script change; push for undo here
		// as we don't want to keep pushing for all input.
		if (m_document)
			m_document->push();

		if (m_scriptManager)
		{
			// Take snapshot of script and try to compile it.
			std::wstring script;
			std::set< std::wstring > usings;
			m_preprocessor->evaluate(m_edit->getText(), script, usings);

			if (m_scriptManager->compile(L"", script, this))
			{
				// Reset error status.
				m_compileStatus->setText(i18n::Text(L"SCRIPT_EDITOR_STATUS_READY"));
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
}
