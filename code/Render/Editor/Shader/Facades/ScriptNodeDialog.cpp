#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/Splitter.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageHlsl.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const wchar_t* c_parameterTypes[] =
{
	L"SHADERGRAPH_SCRIPT_TYPE_SCALAR",	// PtScalar
	L"SHADERGRAPH_SCRIPT_TYPE_VECTOR",
	L"SHADERGRAPH_SCRIPT_TYPE_MATRIX",
	L"SHADERGRAPH_SCRIPT_TYPE_TEXTURE_2D",
	L"SHADERGRAPH_SCRIPT_TYPE_TEXTURE_3D",
	L"SHADERGRAPH_SCRIPT_TYPE_TEXTURE_CUBE",
	L"SHADERGRAPH_SCRIPT_TYPE_STRUCT_BUFFER"
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
		ui::dpi96(1100),
		ui::dpi96(800),
		ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton,
		new ui::FloodLayout())
	)
		return false;

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, ui::dpi96(230));

	Ref< ui::Splitter > splitter2 = new ui::Splitter();
	splitter2->create(splitter, false, ui::dpi96(-20), true);

	m_inputPinList = new ui::GridView();
	m_inputPinList->create(splitter2, ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit| ui::WsDoubleBuffer);
	m_inputPinList->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_SCRIPT_INPUT"), ui::dpi96(90), true));
	m_inputPinList->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_SCRIPT_TYPE"), ui::dpi96(70), false));
	m_inputPinList->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_SCRIPT_SAMPLER"), ui::dpi96(110), false));
	m_inputPinList->addEventHandler< ui::GridRowDoubleClickEvent >(this, &ScriptNodeDialog::eventInputPinRowDoubleClick);
	m_inputPinList->addEventHandler< ui::GridItemContentChangeEvent >(this, &ScriptNodeDialog::eventInputPinEdit);

	int32_t inputPinCount = m_script->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem(m_script->getInputPin(i)->getName()));
		row->add(new ui::GridItem(i18n::Text(c_parameterTypes[m_script->getInputPinType(i)])));
		row->add(new ui::GridItem(m_script->getInputPinSamplerId(i)));
		m_inputPinList->addRow(row);
	}

	Ref< ui::GridRow > lastInputRow = new ui::GridRow();
	lastInputRow->add(new ui::GridItem(i18n::Text(L"SHADERGRAPH_SCRIPT_ADD_PIN")));
	lastInputRow->add(new ui::GridItem(L""));
	lastInputRow->add(new ui::GridItem(L""));
	m_inputPinList->addRow(lastInputRow);

	m_outputPinList = new ui::GridView();
	m_outputPinList->create(splitter2, ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit | ui::WsDoubleBuffer);
	m_outputPinList->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_SCRIPT_OUTPUT"), ui::dpi96(90), true));
	m_outputPinList->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_SCRIPT_TYPE"), ui::dpi96(70), false));
	m_outputPinList->addEventHandler< ui::GridRowDoubleClickEvent >(this, &ScriptNodeDialog::eventOutputPinRowDoubleClick);
	m_outputPinList->addEventHandler< ui::GridItemContentChangeEvent >(this, &ScriptNodeDialog::eventOutputPinEdit);

	int32_t outputPinCount = m_script->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem(m_script->getOutputPin(i)->getName()));
		row->add(new ui::GridItem(i18n::Text(c_parameterTypes[m_script->getOutputPinType(i)])));
		m_outputPinList->addRow(row);
	}

	Ref< ui::GridRow > lastOutputRow = new ui::GridRow();
	lastOutputRow->add(new ui::GridItem(i18n::Text(L"SHADERGRAPH_SCRIPT_ADD_PIN")));
	lastOutputRow->add(new ui::GridItem(L""));
	m_outputPinList->addRow(lastOutputRow);

	m_edit = new ui::SyntaxRichEdit();
	if (!m_edit->create(splitter, m_script->getScript(), ui::WsDoubleBuffer))
		return false;

	m_edit->setLanguage(new ui::SyntaxLanguageHlsl());

	std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
	int32_t fontSize = m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11);
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
	ui::GridRow* row = m_inputPinList->getRow(index);
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
	ui::GridRow* row = m_outputPinList->getRow(index);
	T_FATAL_ASSERT (row);

	return row->get(0)->getText();
}

ParameterType ScriptNodeDialog::getOutputPinType(int32_t index) const
{
	return PtScalar;
}

bool ScriptNodeDialog::apply(ShaderGraph* shaderGraph, Script* script, bool& outCompleteRebuild) const
{
	outCompleteRebuild = false;

	// Update script in shader node.
	script->setScript(getText());

	// Remove input edges which are connected to pins which are no longer available.
	std::set< std::wstring > removeInputPins;
	for (int32_t i = 0; i < script->getInputPinCount(); ++i)
	{
		const InputPin* inputPin = script->getInputPin(i);
		bool pinExist = false;
		for (int32_t j = 0; j < getInputPinCount(); ++j)
		{
			if (inputPin->getName() == getInputPinName(j))
			{
				pinExist = true;
				break;
			}
		}
		if (!pinExist)
		{
			Edge* inputEdge = shaderGraph->findEdge(inputPin);
			if (inputEdge)
			{
				shaderGraph->removeEdge(inputEdge);
				outCompleteRebuild = true;
			}
			removeInputPins.insert(inputPin->getName());
		}
	}
	for (const auto& pin : removeInputPins)
		script->removeInputPin(pin);

	// Remove output edges which are connected to pins which are no longer available.
	std::set< std::wstring > removeOutputPins;
	for (int32_t i = 0; i < script->getOutputPinCount(); ++i)
	{
		const OutputPin* outputPin = script->getOutputPin(i);
		bool pinExist = false;
		for (int32_t j = 0; j < getOutputPinCount(); ++j)
		{
			if (outputPin->getName() == getOutputPinName(j))
			{
				pinExist = true;
				break;
			}
		}
		if (!pinExist)
		{
			RefSet< Edge > outputEdges;
			shaderGraph->findEdges(outputPin, outputEdges);
			if (!outputEdges.empty())
			{
				for (auto outputEdge : outputEdges)
					shaderGraph->removeEdge(outputEdge);
				outCompleteRebuild = true;
			}
			removeOutputPins.insert(outputPin->getName());
		}
	}
	for (const auto& pin : removeOutputPins)
		script->removeOutputPin(pin);			

	// Add new input pins.
	for (int32_t i = 0; i < getInputPinCount(); ++i)
	{
		bool pinExist = false;
		for (int32_t j = 0; j < script->getInputPinCount(); ++j)
		{
			const InputPin* inputPin = script->getInputPin(j);
			if (inputPin->getName() == getInputPinName(i))
			{
				pinExist = true;
				break;
			}
		}
		if (!pinExist)
		{
			const Guid c_null;
			std::wstring pinName = getInputPinName(i);
			ParameterType pinType = getInputPinType(i);
			script->addInputPin(c_null, pinName, pinType);
			outCompleteRebuild = true;
		}
	}

	// Add new output pins.
	for (int32_t i = 0; i < getOutputPinCount(); ++i)
	{
		bool pinExist = false;
		for (int32_t j = 0; j < script->getOutputPinCount(); ++j)
		{
			const OutputPin* outputPin = script->getOutputPin(j);
			if (outputPin->getName() == getOutputPinName(i))
			{
				pinExist = true;
				break;
			}
		}
		if (!pinExist)
		{
			const Guid c_null;
			std::wstring pinName = getOutputPinName(i);
			ParameterType pinType = getOutputPinType(i);
			script->addOutputPin(c_null, pinName, pinType);
			outCompleteRebuild = true;
		}
	}

	return true;
}

void ScriptNodeDialog::eventInputPinRowDoubleClick(ui::GridRowDoubleClickEvent* event)
{
	ui::GridRow* row = event->getRow();
	if (m_inputPinList->getRows().back() != row)
		return;

	row->setEditable(true);
	row->set(0, new ui::GridItem(i18n::Text(L"SHADERGRAPH_SCRIPT_UNNAMED")));
	row->set(1, new ui::GridItem(i18n::Text(c_parameterTypes[0])));
	row->set(2, new ui::GridItem(L""));

	Ref< ui::GridRow > lastInputRow = new ui::GridRow();
	lastInputRow->setEditable(false);
	lastInputRow->add(new ui::GridItem(i18n::Text(L"SHADERGRAPH_SCRIPT_ADD_PIN")));
	lastInputRow->add(new ui::GridItem(L""));
	lastInputRow->add(new ui::GridItem(L""));
	m_inputPinList->addRow(lastInputRow);
}

void ScriptNodeDialog::eventInputPinEdit(ui::GridItemContentChangeEvent* event)
{
	event->consume();
}

void ScriptNodeDialog::eventOutputPinRowDoubleClick(ui::GridRowDoubleClickEvent* event)
{
	ui::GridRow* row = event->getRow();
	if (m_outputPinList->getRows().back() != row)
		return;

	row->setEditable(true);
	row->set(0, new ui::GridItem(i18n::Text(L"SHADERGRAPH_SCRIPT_UNNAMED")));
	row->set(1, new ui::GridItem(i18n::Text(c_parameterTypes[0])));

	Ref< ui::GridRow > lastOutputRow = new ui::GridRow();
	lastOutputRow->setEditable(false);
	lastOutputRow->add(new ui::GridItem(i18n::Text(L"SHADERGRAPH_SCRIPT_ADD_PIN")));
	lastOutputRow->add(new ui::GridItem(L""));
	m_outputPinList->addRow(lastOutputRow);
}

void ScriptNodeDialog::eventOutputPinEdit(ui::GridItemContentChangeEvent* event)
{
	event->consume();
}

	}
}
