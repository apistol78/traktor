/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/ColorNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/InputNodeShape.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ColorNodeFacade", ColorNodeFacade, INodeFacade)

ColorNodeFacade::ColorNodeFacade()
{
	m_nodeShape = new ui::InputNodeShape();
}

Ref< Node > ColorNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Color();
}

Ref< ui::Node > ColorNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::Node > editorNode = graphControl->createNode(
		i18n::Text(L"SHADERGRAPH_NODE_COLOR"),
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

void ColorNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< Color > colorNode = checked_type_cast< Color* >(shaderNode);

	ui::ColorDialog colorDialog;
	colorDialog.create(
		graphControl,
		i18n::Text(L"COLOR_DIALOG_TEXT"),
		ui::ColorDialog::WsDefaultFixed | ui::ColorDialog::WsAlpha | ui::ColorDialog::WsHDR,
		colorNode->getColor()
	);
	if (colorDialog.showModal() == ui::DialogResult::Ok)
		colorNode->setColor(colorDialog.getColor());
	colorDialog.destroy();
}

void ColorNodeFacade::refreshEditorNode(
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

void ColorNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
