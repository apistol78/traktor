#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Shader/Script.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridItemContentChangeEvent.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridRowDoubleClickEvent.h"
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

ScriptNodeDialog::ScriptNodeDialog(editor::IEditor* editor, Script* script)
:	m_editor(editor)
,	m_script(script)
{
}

bool ScriptNodeDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"SHADERGRAPH_SCRIPT_EDIT"),
		ui::scaleBySystemDPI(1100),
		ui::scaleBySystemDPI(800),
		ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton,
		new ui::FloodLayout())
	)
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, ui::scaleBySystemDPI(230));

	Ref< ui::custom::Splitter > splitter2 = new ui::custom::Splitter();
	splitter2->create(splitter, false, ui::scaleBySystemDPI(-20), true);

	m_inputPinList = new ui::custom::GridView();
	m_inputPinList->create(splitter2, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_inputPinList->addColumn(new ui::custom::GridColumn(L"Input", ui::scaleBySystemDPI(90), true));
	m_inputPinList->addColumn(new ui::custom::GridColumn(L"Type", ui::scaleBySystemDPI(70), false));
	m_inputPinList->addColumn(new ui::custom::GridColumn(L"Sampler", ui::scaleBySystemDPI(110), false));
	m_inputPinList->addEventHandler< ui::custom::GridRowDoubleClickEvent >(this, &ScriptNodeDialog::eventInputPinRowDoubleClick);
	m_inputPinList->addEventHandler< ui::custom::GridItemContentChangeEvent >(this, &ScriptNodeDialog::eventInputPinEdit);

	int32_t inputPinCount = m_script->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(m_script->getInputPin(i)->getName()));
		row->add(new ui::custom::GridItem(c_parameterTypes[m_script->getInputPinType(i)]));
		row->add(new ui::custom::GridItem(m_script->getInputPinSamplerId(i)));
		m_inputPinList->addRow(row);
	}

	Ref< ui::custom::GridRow > lastInputRow = new ui::custom::GridRow();
	lastInputRow->add(new ui::custom::GridItem(L"(Add pin)"));
	lastInputRow->add(new ui::custom::GridItem(L""));
	lastInputRow->add(new ui::custom::GridItem(L""));
	m_inputPinList->addRow(lastInputRow);

	m_outputPinList = new ui::custom::GridView();
	m_outputPinList->create(splitter2, ui::custom::GridView::WsColumnHeader | ui::custom::GridView::WsAutoEdit | ui::WsDoubleBuffer);
	m_outputPinList->addColumn(new ui::custom::GridColumn(L"Output", ui::scaleBySystemDPI(90), true));
	m_outputPinList->addColumn(new ui::custom::GridColumn(L"Type", ui::scaleBySystemDPI(70), false));
	m_outputPinList->addEventHandler< ui::custom::GridRowDoubleClickEvent >(this, &ScriptNodeDialog::eventOutputPinRowDoubleClick);
	m_outputPinList->addEventHandler< ui::custom::GridItemContentChangeEvent >(this, &ScriptNodeDialog::eventOutputPinEdit);

	int32_t outputPinCount = m_script->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(m_script->getOutputPin(i)->getName()));
		row->add(new ui::custom::GridItem(c_parameterTypes[m_script->getOutputPinType(i)]));
		m_outputPinList->addRow(row);
	}

	Ref< ui::custom::GridRow > lastOutputRow = new ui::custom::GridRow();
	lastOutputRow->add(new ui::custom::GridItem(L"(Add pin)"));
	lastOutputRow->add(new ui::custom::GridItem(L""));
	m_outputPinList->addRow(lastOutputRow);

	m_edit = new ui::custom::SyntaxRichEdit();
	if (!m_edit->create(splitter, m_script->getScript(), ui::WsDoubleBuffer))
		return false;

	m_edit->setLanguage(new ui::custom::SyntaxLanguageHlsl());

	std::wstring font = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.Font", L"Consolas");
	int32_t fontSize = m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.FontSize", 14);
	m_edit->setFont(ui::Font(font, fontSize));

	update();
	return true;
}

std::wstring ScriptNodeDialog::getText() const
{
	return m_edit->getText();
}

int32_t ScriptNodeDialog::getInputPinCount() const
{
	return int32_t(m_inputPinList->getRows().size() - 1);
}

std::wstring ScriptNodeDialog::getInputPinName(int32_t index) const
{
	ui::custom::GridRow* row = m_inputPinList->getRow(index);
	T_FATAL_ASSERT (row);

	return row->get(0)->getText();
}

ParameterType ScriptNodeDialog::getInputPinType(int32_t index) const
{
	return PtScalar;
}

int32_t ScriptNodeDialog::getOutputPinCount() const
{
	return int32_t(m_outputPinList->getRows().size() - 1);
}

std::wstring ScriptNodeDialog::getOutputPinName(int32_t index) const
{
	ui::custom::GridRow* row = m_outputPinList->getRow(index);
	T_FATAL_ASSERT (row);

	return row->get(0)->getText();
}

ParameterType ScriptNodeDialog::getOutputPinType(int32_t index) const
{
	return PtScalar;
}

void ScriptNodeDialog::eventInputPinRowDoubleClick(ui::custom::GridRowDoubleClickEvent* event)
{
	ui::custom::GridRow* row = event->getRow();
	if (m_inputPinList->getRows().back() != row)
		return;

	row->set(0, new ui::custom::GridItem(L"Unnamed"));
	row->set(1, new ui::custom::GridItem(c_parameterTypes[0]));
	row->set(2, new ui::custom::GridItem(L""));

	Ref< ui::custom::GridRow > lastInputRow = new ui::custom::GridRow();
	lastInputRow->add(new ui::custom::GridItem(L"(Add pin)"));
	lastInputRow->add(new ui::custom::GridItem(L""));
	lastInputRow->add(new ui::custom::GridItem(L""));
	m_inputPinList->addRow(lastInputRow);
}

void ScriptNodeDialog::eventInputPinEdit(ui::custom::GridItemContentChangeEvent* event)
{
	event->consume();
	//ui::custom::GridItem* item = event->getItem();
	//ui::custom::GridRow* row = item->getRow();
}

void ScriptNodeDialog::eventOutputPinRowDoubleClick(ui::custom::GridRowDoubleClickEvent* event)
{
	ui::custom::GridRow* row = event->getRow();
	if (m_outputPinList->getRows().back() != row)
		return;

	row->set(0, new ui::custom::GridItem(L"Unnamed"));
	row->set(1, new ui::custom::GridItem(c_parameterTypes[0]));

	Ref< ui::custom::GridRow > lastOutputRow = new ui::custom::GridRow();
	lastOutputRow->add(new ui::custom::GridItem(L"(Add pin)"));
	lastOutputRow->add(new ui::custom::GridItem(L""));
	m_outputPinList->addRow(lastOutputRow);
}

void ScriptNodeDialog::eventOutputPinEdit(ui::custom::GridItemContentChangeEvent* event)
{
	event->consume();
}

	}
}
