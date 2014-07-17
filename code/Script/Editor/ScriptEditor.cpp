#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Text.h"
#include "Script/CallStack.h"
#include "Script/IScriptProfiler.h"
#include "Script/Editor/IScriptOutline.h"
#include "Script/Editor/Preprocessor.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptBreakpointEvent.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Script/Editor/ScriptEditor.h"
#include "Script/Editor/ScriptProfilerView.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"
#include "Ui/Custom/InputDialog.h"
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

// Resources
#include "Resources/Editor.h"
#include "Resources/PlusMinus.h"
#include "Resources/ScriptFunction.h"
#include "Resources/ScriptFunctionReference.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptEditor", ScriptEditor, editor::IObjectEditor)

ScriptEditor::ScriptEditor(editor::IEditor* editor)
:	m_editor(editor)
,	m_compileCountDown(0)
{
	m_bitmapFunction = ui::Bitmap::load(c_ResourceScriptFunction, sizeof(c_ResourceScriptFunction), L"png");
	m_bitmapFunctionReference = ui::Bitmap::load(c_ResourceScriptFunctionReference, sizeof(c_ResourceScriptFunctionReference), L"png");
}

bool ScriptEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;

	m_script = dynamic_type_cast< Script* >(object);
	if (!m_script)
		return false;

	m_splitter = new ui::custom::Splitter();
	if (!m_splitter->create(parent, true, 250))
		return false;

	Ref< ui::Tab > tab = new ui::Tab();
	if (!tab->create(m_splitter, ui::WsNone))
		return false;

	Ref< ui::TabPage > tabOutline = new ui::TabPage();
	if (!tabOutline->create(tab, L"Outline", new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_outlineGrid = new ui::custom::GridView();
	if (!m_outlineGrid->create(tabOutline, ui::custom::GridView::WsColumnHeader | ui::WsClientBorder | ui::WsDoubleBuffer))
		return false;
	m_outlineGrid->addColumn(new ui::custom::GridColumn(L"", 30));
	m_outlineGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_NAME"), 165));
	m_outlineGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_LINE"), 45));
	m_outlineGrid->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptEditor::eventOutlineDoubleClick);

	Ref< ui::TabPage > tabDependencies = new ui::TabPage();
	if (!tabDependencies->create(tab, L"Dependencies", new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::custom::ToolBar > dependencyTools = new ui::custom::ToolBar();
	if (!dependencyTools->create(tabDependencies))
		return false;

	dependencyTools->addImage(ui::Bitmap::load(c_ResourcePlusMinus, sizeof(c_ResourcePlusMinus), L"png"), 4);
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_ADD_DEPENDENCY"), 0, ui::Command(L"Script.Editor.AddDependency")));
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_REMOVE_DEPENDENCY"), 1, ui::Command(L"Script.Editor.RemoveDependency")));
	dependencyTools->addItem(new ui::custom::ToolBarSeparator());
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_UP"), 2, ui::Command(L"Script.Editor.MoveDependencyUp")));
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_DOWN"), 3, ui::Command(L"Script.Editor.MoveDependencyDown")));
	dependencyTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ScriptEditor::eventDependencyToolClick);

	m_dependencyList = new ui::ListBox();
	if (!m_dependencyList->create(tabDependencies))
		return false;

	m_dependencyList->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptEditor::eventDependencyListDoubleClick);

	Ref< ui::TabPage > tabDependents = new ui::TabPage();
	if (!tabDependents->create(tab, L"Dependents", new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_dependentList = new ui::ListBox();
	if (!m_dependentList->create(tabDependents))
		return false;

	m_dependentList->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptEditor::eventDependentListDoubleClick);

	tab->addPage(tabOutline);
	tab->addPage(tabDependencies);
	tab->addPage(tabDependents);
	tab->setActivePage(tabOutline);

	Ref< ui::custom::Splitter > splitterWork = new ui::custom::Splitter();
	splitterWork->create(m_splitter, false, -200);

	Ref< ui::Container > containerEdit = new ui::Container();
	if (!containerEdit->create(splitterWork, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0, 0)))
		return false;

	Ref< ui::custom::ToolBar > toolBarEdit = new ui::custom::ToolBar();
	toolBarEdit->create(containerEdit);
	toolBarEdit->addItem(new ui::custom::ToolBarButton(L"Toggle comments", ui::Command(L"Script.Editor.ToggleComments")));
	toolBarEdit->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ScriptEditor::eventToolBarEditClick);

	m_edit = new ui::custom::SyntaxRichEdit();
	if (!m_edit->create(containerEdit, m_script->getText()))
		return false;

	m_edit->addImage(ui::Bitmap::load(c_ResourceEditor, sizeof(c_ResourceEditor), L"png"), 1);

#if defined(__APPLE__)
	m_edit->setFont(ui::Font(L"Courier New", 14));
#elif defined(__LINUX__)
	m_edit->setFont(ui::Font(L"Courier New", 14));
#else
	m_edit->setFont(ui::Font(L"Consolas", 14));
#endif
	m_edit->addEventHandler< ui::ContentChangeEvent >(this, &ScriptEditor::eventScriptChange);
	m_edit->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptEditor::eventScriptDoubleClick);

	m_compileStatus = new ui::custom::StatusBar();
	if (!m_compileStatus->create(containerEdit, ui::WsClientBorder))
		return false;

	Ref< ui::custom::Panel > panelDebugger = new ui::custom::Panel();
	panelDebugger->create(splitterWork, L"Debugger", new ui::FloodLayout());

	m_tabSessions = new ui::Tab();
	m_tabSessions->create(panelDebugger, ui::WsNone);

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
	if (m_scriptManager)
	{
		parent->addEventHandler< ui::TimerEvent >(this, &ScriptEditor::eventTimer);
		parent->startTimer(100);
		m_compileCountDown = 1;
	}

	// Create preprocessor.
	m_preprocessor = new Preprocessor();

	// Get debugger implementation.
	m_scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (m_scriptDebuggerSessions)
		m_scriptDebuggerSessions->addListener(this);

	updateDependencyList();
	updateDependentList();
	return true;
}

void ScriptEditor::destroy()
{
	if (m_scriptDebuggerSessions != 0)
	{
		m_scriptDebuggerSessions->removeListener(this);
		m_scriptDebuggerSessions = 0;
	}

	m_preprocessor = 0;

	safeDestroy(m_scriptManager);
	safeDestroy(m_splitter);
}

void ScriptEditor::apply()
{
	m_script->setText(m_edit->getText());
	m_instance->setObject(m_script);
}

bool ScriptEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Find")
	{
		ui::custom::InputDialog::Field fields[] =
		{
			{ L"Find", L"", 0, 0 }
		};

		Ref< ui::custom::InputDialog > dialogFind = new ui::custom::InputDialog();
		dialogFind->create(m_edit, L"Find", L"Enter text or word to search for", fields, sizeof_array(fields));
		if (dialogFind->showModal() == ui::DrOk)
		{
			if (!(m_findNeedle = fields[0].value).empty())
			{
				int32_t caretLine = m_edit->getLineFromOffset(m_edit->getCaretOffset());
				int32_t line = caretLine;

				while (line < m_edit->getLineCount())
				{
					std::wstring text = m_edit->getLine(line);
					if (text.find(m_findNeedle) != text.npos)
					{
						m_edit->showLine(line);
						m_edit->placeCaret(m_edit->getLineOffset(line));
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
						if (text.find(m_findNeedle) != text.npos)
						{
							m_edit->showLine(line);
							m_edit->placeCaret(m_edit->getLineOffset(line));
							break;
						}
						++line;
					}
				}
			}
		}
	}
	else if (command == L"Editor.FindNext")
	{
		if (!m_findNeedle.empty())
		{
			int32_t caretLine = m_edit->getLineFromOffset(m_edit->getCaretOffset());
			int32_t line = caretLine + 1;

			while (line < m_edit->getLineCount())
			{
				std::wstring text = m_edit->getLine(line);
				if (text.find(m_findNeedle) != text.npos)
				{
					m_edit->showLine(line);
					m_edit->placeCaret(m_edit->getLineOffset(line));
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
					if (text.find(m_findNeedle) != text.npos)
					{
						m_edit->showLine(line);
						m_edit->placeCaret(m_edit->getLineOffset(line));
						break;
					}
					++line;
				}
			}
		}
	}
	else if (command == L"Editor.Replace")
	{
		ui::custom::InputDialog::Field fields[] =
		{
			{ L"Search", L"", 0, 0 },
			{ L"Replace with", L"", 0, 0 },
		};

		Ref< ui::custom::InputDialog > dialogReplace = new ui::custom::InputDialog();
		dialogReplace->create(m_edit, L"Replace", L"Enter text or word to replace", fields, sizeof_array(fields));
		if (dialogReplace->showModal() == ui::DrOk)
		{
		}
	}
	else if (command == L"Editor.ReplaceAll")
	{
		ui::custom::InputDialog::Field fields[] =
		{
			{ L"Search", L"", 0, 0 },
			{ L"Replace with", L"", 0, 0 },
		};

		Ref< ui::custom::InputDialog > dialogReplace = new ui::custom::InputDialog();
		dialogReplace->create(m_edit, L"Replace All", L"Enter text or word to replace", fields, sizeof_array(fields));
		if (dialogReplace->showModal() == ui::DrOk)
		{
			if (
				!(m_findNeedle = fields[0].value).empty() &&
				!(m_replaceValue = fields[1].value).empty()
			)
			{
				for (int32_t line = 0; line < m_edit->getLineCount(); ++line)
				{
					std::wstring text = m_edit->getLine(line);
					m_edit->setLine(line, replaceAll(text, m_findNeedle, m_replaceValue));
				}
			}
		}
	}
	else
	{
		ui::TabPage* tabPageSession = m_tabSessions->getActivePage();
		if (tabPageSession)
		{
			Ref< ScriptDebuggerView > debuggerView = tabPageSession->getData< ScriptDebuggerView >(L"VIEW");
			T_ASSERT (debuggerView);

			if (debuggerView->handleCommand(command))
				return true;
		}
		return false;
	}

	return true;
}

void ScriptEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size ScriptEditor::getPreferredSize() const
{
	return ui::Size(1000, 800);
}

void ScriptEditor::syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message)
{
	StringOutputStream ss;
	ss << L"Syntax error (" << line << L") : " << message;
	m_compileStatus->setText(ss.str());
	if (line > 0)
		m_edit->setErrorHighlight(line - 1);
}

void ScriptEditor::otherError(const std::wstring& message)
{
	StringOutputStream ss;
	ss << L"Error : " << message;
	m_compileStatus->setText(ss.str());
}

void ScriptEditor::notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	Ref< ui::TabPage > tabPageSession = new ui::TabPage();
	tabPageSession->create(m_tabSessions, L"Session 0", new ui::FloodLayout());

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(tabPageSession, true, 70, true);

	Ref< ScriptDebuggerView > debuggerView = new ScriptDebuggerView(scriptDebugger);
	debuggerView->create(splitter);
	debuggerView->addEventHandler< ScriptBreakpointEvent >(this, &ScriptEditor::eventBreakPoint);

	Ref< ScriptProfilerView > profilerView = new ScriptProfilerView(scriptProfiler);
	profilerView->create(splitter);

	tabPageSession->setData(L"DEBUGGER", scriptDebugger);
	tabPageSession->setData(L"PROFILER", scriptProfiler);
	tabPageSession->setData(L"VIEW", debuggerView);

	m_tabSessions->addPage(tabPageSession);
	m_tabSessions->setActivePage(tabPageSession);
	m_tabSessions->update();
}

void ScriptEditor::notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	int32_t pageCount = m_tabSessions->getPageCount();
	for (int32_t i = 0; i < pageCount; ++i)
	{
		ui::TabPage* tabPageSession = m_tabSessions->getPage(i);
		T_ASSERT (tabPageSession);

		if (tabPageSession->getData< IScriptDebugger >(L"DEBUGGER") == scriptDebugger)
		{
			m_tabSessions->removePage(tabPageSession);
			safeDestroy(tabPageSession);
			break;
		}
	}
	m_tabSessions->update();
}

void ScriptEditor::notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	if (scriptId == m_instance->getGuid())
	{
		m_edit->setImage(lineNumber, 0);
		m_edit->update();
	}
}

void ScriptEditor::notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	if (scriptId == m_instance->getGuid())
	{
		m_edit->setImage(lineNumber, 1);
		m_edit->update();
	}
}

void ScriptEditor::updateDependencyList()
{
	m_dependencyList->removeAll();

	const std::vector< Guid >& dependencies = m_script->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(*i);
		if (scriptInstance)
			m_dependencyList->add(scriptInstance->getPath());
		else
			m_dependencyList->add(i->format());
	}
}

void ScriptEditor::updateDependentList()
{
	m_dependentList->removeAll();

	RefArray< db::Instance > scriptInstances;
	db::recursiveFindChildInstances(m_editor->getSourceDatabase()->getRootGroup(), db::FindInstanceByType(type_of< Script >()), scriptInstances);

	for (RefArray< db::Instance >::const_iterator i = scriptInstances.begin(); i != scriptInstances.end(); ++i)
	{
		Ref< const Script > scriptObject = (*i)->getObject< const Script >();
		if (scriptObject)
		{
			const std::vector< Guid >& scriptDependencies = scriptObject->getDependencies();
			if (std::find(scriptDependencies.begin(), scriptDependencies.end(), m_instance->getGuid()) != scriptDependencies.end())
			{
				m_dependentList->add((*i)->getPath(), *i);
			}
		}
	}
}

void ScriptEditor::buildOutlineGrid(ui::custom::GridView* grid, ui::custom::GridRow* parent, const IScriptOutline::Node* on)
{
	while (on)
	{
		if (const IScriptOutline::FunctionNode* fn = dynamic_type_cast< const IScriptOutline::FunctionNode* >(on))
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);

			row->add(new ui::custom::GridItem(m_bitmapFunction));
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

void ScriptEditor::eventOutlineDoubleClick(ui::MouseDoubleClickEvent* event)
{
	const ui::custom::GridRow* selectedRow = m_outlineGrid->getSelectedRow();
	if (!selectedRow)
		return;

	const ui::custom::GridItem* lineItem = checked_type_cast< const ui::custom::GridItem*, false >(selectedRow->get(2));
	int32_t line = parseString< int32_t >(lineItem->getText()) - 1;
	if (line >= 0)
	{
		m_edit->showLine(line);
		m_edit->placeCaret(m_edit->getLineOffset(line));
	}
}

void ScriptEditor::eventDependencyToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Script.Editor.AddDependency")
	{
		editor::TypeBrowseFilter filter(type_of< Script >());
		Ref< db::Instance > scriptInstance = m_editor->browseInstance(&filter);
		if (scriptInstance)
		{
			m_script->addDependency(scriptInstance->getGuid());
			updateDependencyList();
		}
	}
	else if (cmd == L"Script.Editor.RemoveDependency")
	{
		int32_t selectedIndex = m_dependencyList->getSelected();
		if (selectedIndex >= 0)
		{
			std::vector< Guid >& dependencies = m_script->getDependencies();
			dependencies.erase(dependencies.begin() + selectedIndex);
			updateDependencyList();
		}
	}
	else if (cmd == L"Script.Editor.MoveDependencyUp")
	{
		int32_t selectedIndex = m_dependencyList->getSelected();
		if (selectedIndex > 0)
		{
			std::vector< Guid >& dependencies = m_script->getDependencies();
			std::swap(dependencies[selectedIndex - 1], dependencies[selectedIndex]);
			updateDependencyList();
			m_dependencyList->select(selectedIndex - 1);
		}
	}
	else if (cmd == L"Script.Editor.MoveDependencyDown")
	{
		int32_t selectedIndex = m_dependencyList->getSelected();
		if (selectedIndex < m_dependencyList->count() -1 )
		{
			std::vector< Guid >& dependencies = m_script->getDependencies();
			std::swap(dependencies[selectedIndex + 1], dependencies[selectedIndex]);
			updateDependencyList();
			m_dependencyList->select(selectedIndex + 1);
		}
	}
}

void ScriptEditor::eventDependencyListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	int selectedIndex = m_dependencyList->getSelected();
	if (selectedIndex >= 0)
	{
		const std::vector< Guid >& dependencies = m_script->getDependencies();
		Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(dependencies[selectedIndex]);
		if (scriptInstance)
			m_editor->openEditor(scriptInstance);
	}
}

void ScriptEditor::eventDependentListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	int selectedIndex = m_dependentList->getSelected();
	if (selectedIndex >= 0)
	{
		Ref< db::Instance > scriptInstance = m_dependentList->getData< db::Instance >(selectedIndex);
		if (scriptInstance)
			m_editor->openEditor(scriptInstance);
	}
}

void ScriptEditor::eventToolBarEditClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"Script.Editor.ToggleComments")
	{
		int32_t startOffset = m_edit->getSelectionStartOffset();
		int32_t stopOffset = m_edit->getSelectionStopOffset();
		if (startOffset >= 0 && stopOffset >= 0)
		{
			std::wstring lineComment = m_edit->getLanguage()->lineComment();
			T_ASSERT (!lineComment.empty());

			int32_t startLine = m_edit->getLineFromOffset(startOffset);
			int32_t stopLine = m_edit->getLineFromOffset(stopOffset);

			for (int32_t i = startLine; i < stopLine; ++i)
			{
				std::wstring line = m_edit->getLine(i);
				if (startsWith(line, lineComment))
					line = line.substr(2);
				else
					line = lineComment + line;
				m_edit->setLine(i, line);
			}

			m_edit->updateLanguage(startLine, stopLine);
			m_edit->update();
		}
	}
}

void ScriptEditor::eventScriptChange(ui::ContentChangeEvent* event)
{
	m_compileCountDown = 10;
	m_compileStatus->setText(L"");
}

void ScriptEditor::eventScriptDoubleClick(ui::MouseDoubleClickEvent* event)
{
	int32_t offset = m_edit->getCaretOffset();
	int32_t line = m_edit->getLineFromOffset(offset);

	if (line < 0)
		return;

	if (!m_scriptDebuggerSessions->haveBreakpoint(m_instance->getGuid(), line))
		m_scriptDebuggerSessions->setBreakpoint(m_instance->getGuid(), line);
	else
		m_scriptDebuggerSessions->removeBreakpoint(m_instance->getGuid(), line);
}

void ScriptEditor::eventTimer(ui::TimerEvent* event)
{
	T_ASSERT (m_scriptManager);

	if (--m_compileCountDown == 0 && m_scriptManager)
	{
		// Take snapshot of script and try to compile it.
		std::wstring script;
		m_preprocessor->evaluate(m_edit->getText(), script);

		if (m_scriptManager->compile(L"", script, 0, this))
		{
			// Reset error status.
			m_compileStatus->setText(L"");
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

void ScriptEditor::eventBreakPoint(ScriptBreakpointEvent* event)
{
	const CallStack* callStack = event->getCallStack();
	const CallStack::Frame& currentFrame = callStack->getCurrentFrame();

	if (currentFrame.scriptId == m_instance->getGuid())
	{
		m_edit->showLine(currentFrame.line);
		m_edit->placeCaret(m_edit->getLineOffset(currentFrame.line));
	}
}

	}
}
