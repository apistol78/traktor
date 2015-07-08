#include "I18N/Text.h"
#include "Render/Shader/Script.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageHlsl.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const wchar_t* c_parameterTypes[] =
{
	L"Scalar",
	L"Vector",
	L"Matrix",
	L"Texture 2D",
	L"Texture 3D",
	L"Texture Cube"
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeDialog", ScriptNodeDialog, ui::ConfigDialog)

ScriptNodeDialog::ScriptNodeDialog(Script* script)
:	m_script(script)
{
}

bool ScriptNodeDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"SHADERGRAPH_SCRIPT_EDIT"), 1100, 800, ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton, new ui::FloodLayout()))
		return false;

	addEventHandler< ui::ButtonClickEvent >(this, &ScriptNodeDialog::eventClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 230);

	Ref< ui::custom::Splitter > splitter2 = new ui::custom::Splitter();
	splitter2->create(splitter, false, -20, true);

	m_inputPinList = new ui::custom::GridView();
	m_inputPinList->create(splitter2, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_inputPinList->addColumn(new ui::custom::GridColumn(L"Input", 90));
	m_inputPinList->addColumn(new ui::custom::GridColumn(L"Type", 70));
	m_inputPinList->addColumn(new ui::custom::GridColumn(L"Sampler", 110));

	int32_t inputPinCount = m_script->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(m_script->getInputPin(i)->getName()));
		row->add(new ui::custom::GridItem(c_parameterTypes[m_script->getInputPinType(i)]));
		row->add(new ui::custom::GridItem(m_script->getInputPinSamplerId(i)));
		m_inputPinList->addRow(row);
	}

	m_outputPinList = new ui::custom::GridView();
	m_outputPinList->create(splitter2, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_outputPinList->addColumn(new ui::custom::GridColumn(L"Output", 90));
	m_outputPinList->addColumn(new ui::custom::GridColumn(L"Type", 70));

	int32_t outputPinCount = m_script->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(m_script->getOutputPin(i)->getName()));
		row->add(new ui::custom::GridItem(c_parameterTypes[m_script->getOutputPinType(i)]));
		m_outputPinList->addRow(row);
	}

	m_edit = new ui::custom::SyntaxRichEdit();
	if (!m_edit->create(splitter, m_script->getScript(), ui::WsDoubleBuffer))
		return false;

	m_edit->setLanguage(new ui::custom::SyntaxLanguageHlsl());
#if defined(__APPLE__)
	m_edit->setFont(ui::Font(L"Menlo Regular", 11));
#elif defined(__LINUX__)
	m_edit->setFont(ui::Font(L"Monospace", 11));
#else
	m_edit->setFont(ui::Font(L"Consolas", ui::scaleBySystemDPI(14)));
#endif

	update();
	return true;
}

//void ScriptNodeDialog::eventInputEditEvent(ui::custom::EditListEditEvent* event)
//{
//	if (event->getIndex() < 0)	// Add item.
//	{
//		m_script->addInputPin(event->getText(), render::PtScalar);
//		m_inputPinList->add(event->getText());
//	}
//	else if (event->getText().empty())	// Remove item.
//	{
//		m_script->removeInputPin(event->getText());
//		event->consume();
//	}
//	else	// Rename item.
//	{
//		std::wstring currentName = m_inputPinList->getItem(event->getIndex());
//		const TypedInputPin* currentPin = static_cast< const TypedInputPin* >(m_script->findInputPin(currentName));
//		if (currentPin)
//		{
//			ParameterType pinType = currentPin->getType();
//			m_script->removeInputPin(currentName);
//			m_script->addInputPin(currentName, pinType);
//		}
//		event->consume();
//	}
//}
//
//void ScriptNodeDialog::eventOutputEditEvent(ui::custom::EditListEditEvent* event)
//{
//	if (event->getIndex() < 0)	// Add item.
//	{
//		m_script->addOutputPin(event->getText(), render::PtScalar);
//		m_outputPinList->add(event->getText());
//	}
//	else if (event->getText().empty())	// Remove item.
//	{
//		m_script->removeOutputPin(event->getText());
//		event->consume();
//	}
//	else	// Rename item.
//	{
//		std::wstring currentName = m_outputPinList->getItem(event->getIndex());
//		const TypedOutputPin* currentPin = static_cast< const TypedOutputPin* >(m_script->findOutputPin(currentName));
//		if (currentPin)
//		{
//			ParameterType pinType = currentPin->getType();
//			m_script->removeOutputPin(currentName);
//			m_script->addOutputPin(currentName, pinType);
//		}
//		event->consume();
//	}
//}

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
