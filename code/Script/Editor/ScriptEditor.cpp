#include "Script/Editor/ScriptEditor.h"
#include "Script/Script.h"
#include "Editor/Editor.h"
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
#include "I18N/Text.h"
#include "Core/Heap/HeapNew.h"

// Resources
#include "Resources/PlusMinus.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptEditor", ScriptEditor, editor::ObjectEditor)

ScriptEditor::ScriptEditor(editor::Editor* editor)
:	m_editor(editor)
{
}

bool ScriptEditor::create(ui::Widget* parent, db::Instance* instance, Object* object)
{
	m_script = dynamic_type_cast< Script* >(object);
	if (!m_script)
		return false;

	m_splitter = gc_new< ui::custom::Splitter >();
	if (!m_splitter->create(parent, true, 150))
		return false;

	Ref< ui::Container > container = gc_new< ui::Container >();
	if (!container->create(m_splitter, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::custom::ToolBar > dependencyTools = gc_new< ui::custom::ToolBar >();
	if (!dependencyTools->create(container))
		return false;

	dependencyTools->addImage(ui::Bitmap::load(c_ResourcePlusMinus, sizeof(c_ResourcePlusMinus), L"png"), 4);
	dependencyTools->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCRIPT_EDITOR_ADD_DEPENDENCY"), ui::Command(L"Script.Editor.AddDependency"), 0));
	dependencyTools->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCRIPT_EDITOR_REMOVE_DEPENDENCY"), ui::Command(L"Script.Editor.RemoveDependency"), 1));
	dependencyTools->addItem(gc_new< ui::custom::ToolBarSeparator >());
	dependencyTools->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_UP"), ui::Command(L"Script.Editor.MoveDependencyUp"), 2));
	dependencyTools->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCRIPT_EDITOR_MOVE_DEPENDENCY_DOWN"), ui::Command(L"Script.Editor.MoveDependencyDown"), 3));
	dependencyTools->addClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventDependencyToolClick));

	m_dependencyList = gc_new< ui::ListBox >();
	if (!m_dependencyList->create(container))
		return false;

	m_dependencyList->addDoubleClickEventHandler(ui::createMethodHandler(this, &ScriptEditor::eventDependencyListDoubleClick));

	m_edit = gc_new< ui::custom::SyntaxRichEdit >();
	if (!m_edit->create(m_splitter, m_script->getText()))
		return false;

	m_edit->setFont(ui::Font(
		L"Courier New",
		16
	));

	m_edit->setLanguage(gc_new< ui::custom::SyntaxLanguageLua >());

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
		const std::vector< Guid >& dependencies = m_script->getDependencies();
		Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(dependencies[selectedIndex]);
		if (scriptInstance)
			m_editor->openEditor(scriptInstance);
	}
}

	}
}
