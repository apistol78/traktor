#include "Script/Editor/ScriptEditor.h"
#include "Script/IScriptManager.h"
#include "Script/Script.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/Settings.h"
#include "Editor/TypeBrowseFilter.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/ListBox.h"
#include "Ui/TableLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageLua.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "I18N/Text.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"

// Resources
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
	m_script = dynamic_type_cast< Script* >(object);
	if (!m_script)
		return false;

	m_splitter = new ui::custom::Splitter();
	if (!m_splitter->create(parent, true, 150))
		return false;

	Ref< ui::Container > container = new ui::Container();
	if (!container->create(m_splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::custom::ToolBar > dependencyTools = new ui::custom::ToolBar();
	if (!dependencyTools->create(container))
		return false;

	dependencyTools->addImage(ui::Bitmap::load(c_ResourcePlusMinus, sizeof(c_ResourcePlusMinus), L"png"), 4);
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_ADD_DEPENDENCY"), ui::Command(L"Script.Editor.AddDependency"), 0));
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_REMOVE_DEPENDENCY"), ui::Command(L"Script.Editor.RemoveDependency"), 1));
	dependencyTools->addItem(new ui::custom::ToolBarSeparator());
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_UP"), ui::Command(L"Script.Editor.MoveDependencyUp"), 2));
	dependencyTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_DOWN"), ui::Command(L"Script.Editor.MoveDependencyDown"), 3));
	dependencyTools->addClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventDependencyToolClick));

	m_dependencyList = new ui::ListBox();
	if (!m_dependencyList->create(container))
		return false;

	m_dependencyList->addDoubleClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventDependencyListDoubleClick));

	Ref< ui::Container > containerEdit = new ui::Container();
	if (!containerEdit->create(m_splitter, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0)))
		return false;

	m_edit = new ui::custom::SyntaxRichEdit();
	if (!m_edit->create(containerEdit, m_script->getText()))
		return false;

	m_edit->setFont(ui::Font(L"Courier New", 16));
	m_edit->addChangeEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventScriptChange));

	m_compileStatus = new ui::custom::StatusBar();
	if (!m_compileStatus->create(containerEdit, ui::WsClientBorder))
		return false;

	// Create language specific implementations.
	{
		std::wstring syntaxLanguageTypeName = m_editor->getSettings()->getProperty< editor::PropertyString >(L"Editor.SyntaxLanguageType");
		const TypeInfo* syntaxLanguageType = TypeInfo::find(syntaxLanguageTypeName);
		if (syntaxLanguageType)
		{
			Ref< ui::custom::SyntaxLanguage > syntaxLanguage = dynamic_type_cast< ui::custom::SyntaxLanguage* >(syntaxLanguageType->createInstance());
			T_ASSERT (syntaxLanguage);
			m_edit->setLanguage(syntaxLanguage);
		}

		std::wstring scriptManagerTypeName = m_editor->getSettings()->getProperty< editor::PropertyString >(L"Editor.ScriptManagerType");
		const TypeInfo* scriptManagerType = TypeInfo::find(scriptManagerTypeName);
		if (scriptManagerType)
		{
			m_scriptManager = dynamic_type_cast< IScriptManager* >(scriptManagerType->createInstance());
			T_ASSERT (m_scriptManager);

			m_scriptContext = m_scriptManager->createContext();
			if (!m_scriptContext)
			{
				log::warning << L"Failed to create script context; interactive syntax check disabled" << Endl;
				m_scriptManager = 0;
			}
		}
	}

	// Setup compile timer.
	if (m_scriptManager)
	{
		parent->addTimerEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventTimer));
		parent->startTimer(100);
	}

	updateDependencyList();
	return true;
}

void ScriptEditor::destroy()
{
	m_splitter->destroy();
}

void ScriptEditor::apply()
{
	m_script->setText(m_edit->getText());
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

void ScriptEditor::updateDependencyList()
{
	Ref< editor::IProject > project = m_editor->getProject();
	T_ASSERT (project);

	m_dependencyList->removeAll();

	const std::vector< Guid >& dependencies = m_script->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< db::Instance > scriptInstance = project->getSourceDatabase()->getInstance(*i);
		if (scriptInstance)
			m_dependencyList->add(scriptInstance->getName());
		else
			m_dependencyList->add(i->format());
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
		int selectedIndex = m_dependencyList->getSelected();
		if (selectedIndex >= 0)
		{
			std::vector< Guid >& dependencies = m_script->getDependencies();
			dependencies.erase(dependencies.begin() + selectedIndex);
			updateDependencyList();
		}
	}
}

void ScriptEditor::eventDependencyListDoubleClick(ui::Event* event)
{
	int selectedIndex = m_dependencyList->getSelected();
	if (selectedIndex >= 0)
	{
		Ref< editor::IProject > project = m_editor->getProject();
		T_ASSERT (project);

		const std::vector< Guid >& dependencies = m_script->getDependencies();
		Ref< db::Instance > scriptInstance = project->getSourceDatabase()->getInstance(dependencies[selectedIndex]);
		if (scriptInstance)
			m_editor->openEditor(scriptInstance);
	}
}

void ScriptEditor::eventScriptChange(ui::Event* event)
{
	m_compileCountDown = 10;
	m_compileStatus->setText(L"");
}

void ScriptEditor::eventTimer(ui::Event* event)
{
	T_ASSERT (m_scriptManager);

	if (--m_compileCountDown == 0 && m_scriptContext)
	{
		// Take snapshot of script and try to compile it.
		std::wstring script = m_edit->getText();
		if (m_scriptContext->executeScript(script, true, this))
		{
			m_compileStatus->setText(L"");
			m_edit->setErrorHighlight(-1);
		}
	}
}

	}
}
