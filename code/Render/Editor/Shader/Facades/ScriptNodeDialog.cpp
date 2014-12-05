#include "I18N/Text.h"
#include "Render/Shader/Script.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/Custom/EditList.h"
#include "Ui/Custom/EditListEditEvent.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageHlsl.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeDialog", ScriptNodeDialog, ui::ConfigDialog)

ScriptNodeDialog::ScriptNodeDialog(Script* script)
:	m_script(script)
{
}

bool ScriptNodeDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"SHADERGRAPH_SCRIPT_EDIT"), 800, 500, ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton, new ui::FloodLayout()))
		return false;

	addEventHandler< ui::ButtonClickEvent >(this, &ScriptNodeDialog::eventClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 150);

	Ref< ui::custom::Splitter > splitter2 = new ui::custom::Splitter();
	splitter2->create(splitter, false, -20, true);

	m_inputPinList = new ui::custom::EditList();
	m_inputPinList->create(splitter2, ui::ListBox::WsDefault | ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove);
	m_inputPinList->addEventHandler< ui::custom::EditListEditEvent >(this, &ScriptNodeDialog::eventInputEditEvent);

	int32_t inputPinCount = m_script->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
		m_inputPinList->add(m_script->getInputPin(i)->getName());

	m_outputPinList = new ui::custom::EditList();
	m_outputPinList->create(splitter2, ui::ListBox::WsDefault | ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove);
	m_outputPinList->addEventHandler< ui::custom::EditListEditEvent >(this, &ScriptNodeDialog::eventOutputEditEvent);

	int32_t outputPinCount = m_script->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
		m_outputPinList->add(m_script->getOutputPin(i)->getName());

	m_edit = new ui::custom::SyntaxRichEdit();
	if (!m_edit->create(splitter, m_script->getScript()))
		return false;

	m_edit->setLanguage(new ui::custom::SyntaxLanguageHlsl());
	m_edit->setFont(ui::Font(L"Courier New", 14));

	update();
	return true;
}

void ScriptNodeDialog::eventInputEditEvent(ui::custom::EditListEditEvent* event)
{
	if (event->getIndex() < 0)	// Add item.
	{
		m_script->addInputPin(event->getText(), render::PtScalar);
		m_inputPinList->add(event->getText());
	}
	else if (event->getText().empty())	// Remove item.
	{
		m_script->removeInputPin(event->getText());
		event->consume();
	}
	else	// Rename item.
	{
		std::wstring currentName = m_inputPinList->getItem(event->getIndex());
		const TypedInputPin* currentPin = static_cast< const TypedInputPin* >(m_script->findInputPin(currentName));
		if (currentPin)
		{
			ParameterType pinType = currentPin->getType();
			m_script->removeInputPin(currentName);
			m_script->addInputPin(currentName, pinType);
		}
		event->consume();
	}
}

void ScriptNodeDialog::eventOutputEditEvent(ui::custom::EditListEditEvent* event)
{
	if (event->getIndex() < 0)	// Add item.
	{
		m_script->addOutputPin(event->getText(), render::PtScalar);
		m_outputPinList->add(event->getText());
	}
	else if (event->getText().empty())	// Remove item.
	{
		m_script->removeOutputPin(event->getText());
		event->consume();
	}
	else	// Rename item.
	{
		std::wstring currentName = m_outputPinList->getItem(event->getIndex());
		const TypedOutputPin* currentPin = static_cast< const TypedOutputPin* >(m_script->findOutputPin(currentName));
		if (currentPin)
		{
			ParameterType pinType = currentPin->getType();
			m_script->removeOutputPin(currentName);
			m_script->addOutputPin(currentName, pinType);
		}
		event->consume();
	}
}

void ScriptNodeDialog::eventClick(ui::ButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command.getId() == ui::DrOk || command.getId() == ui::DrApply)
	{
		std::wstring script = m_edit->getText();
		m_script->setScript(script);
	}
}

	}
}
