/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/SwitchNodeDialog.h"
#include "Render/Editor/Shader/Facades/SwitchNodeFacade.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SwitchNodeFacade", SwitchNodeFacade, INodeFacade)

SwitchNodeFacade::SwitchNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = new ui::custom::DefaultNodeShape(graphControl);
}

Ref< Node > SwitchNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Switch();
}

Ref< ui::custom::Node > SwitchNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		i18n::Text(L"SHADERGRAPH_NODE_SWITCH"),
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(shaderNode->getComment());

	return editorNode;
}

void SwitchNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	SwitchNodeDialog nodeDialog;
	if (!nodeDialog.create(graphControl, checked_type_cast< Switch* >(shaderNode)))
		return;
	if (nodeDialog.showModal() == ui::DrOk)
	{
	}
	nodeDialog.destroy();
}

void SwitchNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
}

void SwitchNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
