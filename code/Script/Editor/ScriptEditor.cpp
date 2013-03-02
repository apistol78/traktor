#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Text.h"
#include "Script/CallStack.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Script/Editor/ScriptEditor.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/MethodHandler.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/Panel.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageLua.h"
#include "Ui/Custom/StatusBar/StatusBar.h"

// Resources
#include "Resources/Editor.h"
#include "Resources/PlusMinus.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptEditor", ScriptEditor, editor::IObjectEditor)

ScriptEditor::ScriptEditor(editor::IEditor* editor)
:	m_editor(editor)
,	m_compileCountDown(0)
{
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
	m_outlineGrid->addColumn(new ui::custom::GridColumn(L"", 20));
	m_outlineGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_NAME"), 170));
	m_outlineGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_OUTLINE_LINE"), 50));
	m_outlineGrid->addDoubleClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventOutlineDoubleClick));

	Ref< ui::TabPage > tabDependencies = new ui::TabPage();
	if (!tabDependencies->create(tab, L"Dependencies", new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::custom::ToolBar > dependencyTools = new ui::custom::ToolBar();
	if (!dependencyTools->create(tabDependencies))
		return false;

	dependencyTools->addImage(ui::Bitmap::load(c_ResourcePlusMinus, sizeof(c_ResourcePlusMinus), L"png"), 4);
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_ADD_DEPENDENCY"), ui::Command(L"Script.Editor.AddDependency"), 0));
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_REMOVE_DEPENDENCY"), ui::Command(L"Script.Editor.RemoveDependency"), 1));
	dependencyTools->addItem(new ui::custom::ToolBarSeparator());
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_UP"), ui::Command(L"Script.Editor.MoveDependencyUp"), 2));
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_DOWN"), ui::Command(L"Script.Editor.MoveDependencyDown"), 3));
	dependencyTools->addClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventDependencyToolClick));

	m_dependencyList = new ui::ListBox();
	if (!m_dependencyList->create(tabDependencies))
		return false;

	m_dependencyList->addDoubleClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventDependencyListDoubleClick));

	tab->addPage(tabOutline);
	tab->addPage(tabDependencies);
	tab->setActivePage(tabOutline);

	Ref< ui::custom::Splitter > splitterWork = new ui::custom::Splitter();
	splitterWork->create(m_splitter, false, -200);

	Ref< ui::Container > containerEdit = new ui::Container();
	if (!containerEdit->create(splitterWork, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0)))
		return false;

	m_edit = new ui::custom::SyntaxRichEdit();
	if (!m_edit->create(containerEdit, m_script->getText()))
		return false;

	m_edit->addImage(ui::Bitmap::load(c_ResourceEditor, sizeof(c_ResourceEditor), L"png"), 1);

#if defined(__APPLE__)
	m_edit->setFont(ui::Font(L"Courier New", 14));
#else
	m_edit->setFont(ui::Font(L"Courier New", 16));
#endif
	m_edit->addChangeEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventScriptChange));
	m_edit->addDoubleClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventScriptDoubleClick));

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
	}

	// Setup compile timer.
	if (m_scriptManager)
	{
		parent->addTimerEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventTimer));
		parent->startTimer(100);
		m_compileCountDown = 1;
	}

	// Get debugger implementation.
	m_scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (m_scriptDebuggerSessions)
		m_scriptDebuggerSessions->addListener(this);

	updateDependencyList();
	return true;
}

void ScriptEditor::destroy()
{
	if (m_scriptDebuggerSessions != 0)
	{
		m_scriptDebuggerSessions->removeListener(this);
		m_scriptDebuggerSessions = 0;
	}

	m_scriptManager = 0;

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
			{ L"Find", L"", 0 }
		};

		Ref< ui::custom::InputDialog > dialogFind = new ui::custom::InputDialog();
		dialogFind->create(m_edit, L"Find", L"Enter text or word to search for", fields, sizeof_array(fields));
		if (dialogFind->showModal() == ui::DrOk)
		{
			for (int32_t i = 0; i < m_edit->getLineCount(); ++i)
			{
				std::wstring text = m_edit->getLine(i);
				if (text.find(fields[0].value) != text.npos)
				{
					m_edit->scrollToLine(i);
					break;
				}
			}
		}
	}
	else if (command == L"Editor.Replace")
	{
		ui::custom::InputDialog::Field fields[] =
		{
			{ L"Search", L"", 0 },
			{ L"Replace with", L"", 0 },
		};

		Ref< ui::custom::InputDialog > dialogReplace = new ui::custom::InputDialog();
		dialogReplace->create(m_edit, L"Replace", L"Enter text or word to replace", fields, sizeof_array(fields));
		if (dialogReplace->showModal() == ui::DrOk)
		{
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

ui::Size ScriptEditor::getPreferredSize() const
{
	return ui::Size(1000, 800);
}

void ScriptEditor::syntaxError(uint32_t line, const std::wstring& message)
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

void ScriptEditor::notifyDebuggerBeginSession(IScriptDebugger* scriptDebugger)
{
	Ref< ui::TabPage > tabPageSession = new ui::TabPage();
	tabPageSession->create(m_tabSessions, L"Session 0", new ui::FloodLayout());

	Ref< ScriptDebuggerView > debuggerView = new ScriptDebuggerView(scriptDebugger);
	debuggerView->create(tabPageSession);
	debuggerView->addBreakPointEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventBreakPoint));

	tabPageSession->setData(L"DEBUGGER", scriptDebugger);
	tabPageSession->setData(L"VIEW", debuggerView);

	m_tabSessions->addPage(tabPageSession);
	m_tabSessions->setActivePage(tabPageSession);
	m_tabSessions->update();
}

void ScriptEditor::notifyDebuggerEndSession(IScriptDebugger* scriptDebugger)
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

void ScriptEditor::notifyDebuggerSetBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	if (scriptId == m_instance->getGuid())
	{
		m_edit->setImage(lineNumber, 0);
		m_edit->update();
	}
}

void ScriptEditor::notifyDebuggerRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber)
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
			m_dependencyList->add(scriptInstance->getName());
		else
			m_dependencyList->add(i->format());
	}
}

void ScriptEditor::eventOutlineDoubleClick(ui::Event* event)
{
	const ui::custom::GridRow* selectedRow = m_outlineGrid->getSelectedRow();
	if (!selectedRow)
		return;

	const ui::custom::GridItem* lineItem = checked_type_cast< const ui::custom::GridItem*, false >(selectedRow->get(2));
	int32_t line = parseString< int32_t >(lineItem->getText()) - 1;
	if (line >= 0)
	{
		m_edit->scrollToLine(line > 4 ? line - 4 : 0);
		m_edit->placeCaret(m_edit->getLineOffset(line));
	}
}

void ScriptEditor::eventDependencyToolClick(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

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

void ScriptEditor::eventDependencyListDoubleClick(ui::Event* event)
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

void ScriptEditor::eventScriptChange(ui::Event* event)
{
	m_compileCountDown = 10;
	m_compileStatus->setText(L"");
}

void ScriptEditor::eventScriptDoubleClick(ui::Event* event)
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

void ScriptEditor::eventTimer(ui::Event* event)
{
	T_ASSERT (m_scriptManager);

	if (--m_compileCountDown == 0 && m_scriptManager)
	{
		// Take snapshot of script and try to compile it.
		std::wstring script = m_edit->getText();
		if (m_scriptManager->compile(L"", script, 0, this))
		{
			// Reset error status.
			m_compileStatus->setText(L"");
			m_edit->setErrorHighlight(-1);

			// Get outline of script.
			m_outline.clear();
			m_edit->getOutline(m_outline);

			m_outlineGrid->removeAllRows();
			for (std::list< ui::custom::SyntaxOutline >::const_iterator i = m_outline.begin(); i != m_outline.end(); ++i)
			{
				Ref< ui::custom::GridRow > row = new ui::custom::GridRow();

				switch (i->type)
				{
				default:
				case ui::custom::SotUnknown:
					row->add(new ui::custom::GridItem(L"?"));
					break;
				case ui::custom::SotVariable:
					row->add(new ui::custom::GridItem(L"V"));
					break;
				case ui::custom::SotFunction:
					row->add(new ui::custom::GridItem(L"F"));
					break;
				}

				row->add(new ui::custom::GridItem(i->name));
				row->add(new ui::custom::GridItem(toString(i->line + 1)));

				m_outlineGrid->addRow(row);
			}
		}
	}
}

void ScriptEditor::eventBreakPoint(ui::Event* event)
{
	const CallStack* callStack = checked_type_cast< const CallStack* >(event->getItem());
	const CallStack::Frame& currentFrame = callStack->getCurrentFrame();

	if (currentFrame.scriptId == m_instance->getGuid())
	{
		m_edit->scrollToLine(currentFrame.line > 4 ? currentFrame.line - 4 : 0);
		m_edit->placeCaret(m_edit->getLineOffset(currentFrame.line));
	}
}

	}
}
