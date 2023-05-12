/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/Facades/InterpolatorNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/IpolNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.InterpolatorNodeFacade", InterpolatorNodeFacade, INodeFacade)

InterpolatorNodeFacade::InterpolatorNodeFacade()
{
	m_nodeShape = new ui::IpolNodeShape();
}

Ref< Node > InterpolatorNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->createInstance());
}

Ref< ui::Node > InterpolatorNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::Node > editorNode = graphControl->createNode(
		L"",
		L"",
		ui::Point(
			ui::dpi96(shaderNode->getPosition().first),
			ui::dpi96(shaderNode->getPosition().second)
		),
		m_nodeShape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			inputPin->getId(),
			!inputPin->isOptional(),
			false
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName(),
			outputPin->getId()
		);
	}

	editorNode->setComment(shaderNode->getComment());

	return editorNode;
}

void InterpolatorNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
}

void InterpolatorNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
}

void InterpolatorNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
