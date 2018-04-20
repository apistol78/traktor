/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "I18N/Text.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/Script.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class ScriptNodeInstanceData : public Object
{
	T_RTTI_CLASS;

public:
	ScriptNodeInstanceData(
		editor::IEditor* editor,
		ShaderGraphEditorPage* page,
		ui::custom::GraphControl* graphControl,
		ShaderGraph* shaderGraph,
		Script* scriptNode
	)
	:	m_editor(editor)
	,	m_page(page)
	,	m_graphControl(graphControl)
	,	m_shaderGraph(shaderGraph)
	,	m_scriptNode(scriptNode)
	{
	}

	void edit()
	{
		if (m_dialog)
		{
			m_dialog->show();
			return;
		}

		m_dialog = new ScriptNodeDialog(m_editor, m_scriptNode);
		m_dialog->create(m_graphControl);
		m_dialog->show();

		m_dialog->addEventHandler< ui::ButtonClickEvent >(this, &ScriptNodeInstanceData::eventClick);
		m_dialog->addEventHandler< ui::CloseEvent >(this, &ScriptNodeInstanceData::eventCloseDialog);
	}
	
private:
	editor::IEditor* m_editor;
	ShaderGraphEditorPage* m_page;
	ui::custom::GraphControl* m_graphControl;
	ShaderGraph* m_shaderGraph;
	Script* m_scriptNode;
	Ref< ScriptNodeDialog > m_dialog;

	void eventClick(ui::ButtonClickEvent* event)
	{
		const ui::Command& command = event->getCommand();
		if (command.getId() == ui::DrOk || command.getId() == ui::DrApply)
		{
			// Update script in shader node.
			std::wstring script = m_dialog->getText();
			m_scriptNode->setScript(script);

			bool needCompleteRebuild = false;

			// Remove input edges which are connected to pins which are no longer available.
			for (int32_t i = 0; i < m_scriptNode->getInputPinCount(); ++i)
			{
				const InputPin* inputPin = m_scriptNode->getInputPin(i);
				bool pinExist = false;
				for (int32_t j = 0; j < m_dialog->getInputPinCount(); ++j)
				{
					if (inputPin->getName() == m_dialog->getInputPinName(j))
					{
						pinExist = true;
						break;
					}
				}
				if (!pinExist)
				{
					Edge* inputEdge = m_shaderGraph->findEdge(inputPin);
					if (inputEdge)
					{
						m_shaderGraph->removeEdge(inputEdge);
						needCompleteRebuild = true;
					}
				}
			}

			// Remove output edges which are connected to pins which are no longer available.
			for (int32_t i = 0; i < m_scriptNode->getOutputPinCount(); ++i)
			{
				const OutputPin* outputPin = m_scriptNode->getOutputPin(i);
				bool pinExist = false;
				for (int32_t j = 0; j < m_dialog->getOutputPinCount(); ++j)
				{
					if (outputPin->getName() == m_dialog->getOutputPinName(j))
					{
						pinExist = true;
						break;
					}
				}
				if (!pinExist)
				{
					RefSet< Edge > outputEdges;
					m_shaderGraph->findEdges(outputPin, outputEdges);
					if (!outputEdges.empty())
					{
						for (RefSet< Edge >::const_iterator j = outputEdges.begin(); j != outputEdges.end(); ++j)
							m_shaderGraph->removeEdge(*j);
						needCompleteRebuild = true;
					}
				}
			}

			// Add new input pins.
			for (int32_t i = 0; i < m_dialog->getInputPinCount(); ++i)
			{
				bool pinExist = false;
				for (int32_t j = 0; j < m_scriptNode->getInputPinCount(); ++j)
				{
					const InputPin* inputPin = m_scriptNode->getInputPin(j);
					if (inputPin->getName() == m_dialog->getInputPinName(i))
					{
						pinExist = true;
						break;
					}
				}
				if (!pinExist)
				{
					std::wstring pinName = m_dialog->getInputPinName(i);
					ParameterType pinType = m_dialog->getInputPinType(i);
					m_scriptNode->addInputPin(pinName, pinType);
					needCompleteRebuild = true;
				}
			}

			// Add new output pins.
			for (int32_t i = 0; i < m_dialog->getOutputPinCount(); ++i)
			{
				bool pinExist = false;
				for (int32_t j = 0; j < m_scriptNode->getOutputPinCount(); ++j)
				{
					const OutputPin* outputPin = m_scriptNode->getOutputPin(j);
					if (outputPin->getName() == m_dialog->getOutputPinName(i))
					{
						pinExist = true;
						break;
					}
				}
				if (!pinExist)
				{
					std::wstring pinName = m_dialog->getOutputPinName(i);
					ParameterType pinType = m_dialog->getOutputPinType(i);
					m_scriptNode->addOutputPin(pinName, pinType);
					needCompleteRebuild = true;
				}
			}

			// Call back to page to ensure entire editor graph is rebuilt.
			if (needCompleteRebuild)
				m_page->createEditorGraph();
		}
		if (command.getId() == ui::DrOk || command.getId() == ui::DrCancel)
			safeDestroy(m_dialog);
	}

	void eventCloseDialog(ui::CloseEvent* event)
	{
		safeDestroy(m_dialog);
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeInstanceData", ScriptNodeInstanceData, Object)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeFacade", ScriptNodeFacade, INodeFacade)

ScriptNodeFacade::ScriptNodeFacade(ShaderGraphEditorPage* page, ui::custom::GraphControl* graphControl)
:	m_page(page)
{
	m_nodeShape = new ui::custom::DefaultNodeShape(graphControl);
}

Ref< Node > ScriptNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Script();
}

Ref< ui::custom::Node > ScriptNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);

	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		scriptNode->getName(),
		L"",
		ui::Point(
			ui::dpi96(scriptNode->getPosition().first),
			ui::dpi96(scriptNode->getPosition().second)
		),
		m_nodeShape
	);

	for (int j = 0; j < scriptNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = scriptNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < scriptNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = scriptNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(scriptNode->getComment());
	editorNode->setData(L"INSTANCE", new ScriptNodeInstanceData(editor, m_page, graphControl, shaderGraph, scriptNode));

	return editorNode;
}

void ScriptNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	ScriptNodeInstanceData* instanceData = editorNode->getData< ScriptNodeInstanceData >(L"INSTANCE");
	T_ASSERT (instanceData);

	instanceData->edit();
}

void ScriptNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);
	editorNode->setTitle(scriptNode->getName());
	editorNode->setComment(scriptNode->getComment());
}

void ScriptNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
