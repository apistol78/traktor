/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/SwizzleNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/InOutNodeShape.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SwizzleNodeFacade", SwizzleNodeFacade, INodeFacade)

SwizzleNodeFacade::SwizzleNodeFacade()
{
	m_nodeShape = new ui::InOutNodeShape(ui::InOutNodeShape::StDefault);
}

Ref< Node > SwizzleNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->createInstance());
}

Ref< ui::Node > SwizzleNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::Node > editorNode = graphControl->createNode(
		L"",
		shaderNode->getInformation(),
		ui::UnitPoint(
			ui::Unit(shaderNode->getPosition().first),
			ui::Unit(shaderNode->getPosition().second)
		),
		m_nodeShape
	);

	for (int32_t j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			inputPin->getId(),
			!inputPin->isOptional(),
			false
		);
	}

	for (int32_t j = 0; j < shaderNode->getOutputPinCount(); ++j)
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

void SwizzleNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	const ui::Rect rcEditVirtual = graphControl->pixel(editorNode->calculateRect());
	const ui::Rect rcEdit(
		graphControl->virtualToClient(rcEditVirtual.getTopLeft()),
		graphControl->virtualToClient(rcEditVirtual.getBottomRight())
	);

	m_editEditorNode = editorNode;
	m_editShaderNode = mandatory_non_null_type_cast< Swizzle* >(shaderNode);

	if (m_edit == nullptr)
	{
		m_edit = new ui::Edit();
		m_edit->create(graphControl);
		m_edit->addEventHandler< ui::FocusEvent >(
			[this](ui::FocusEvent* event)
			{
				if (m_edit->isVisible(false) && event->lostFocus())
				{
					m_editEditorNode->setInfo(m_edit->getText());
					m_editShaderNode->set(m_edit->getText());
					m_edit->setVisible(false);
				}
			}
		);
		m_edit->addEventHandler< ui::KeyDownEvent >(
			[this](ui::KeyDownEvent* event)
			{
				if (event->getVirtualKey() == ui::VkReturn)
				{
					m_editEditorNode->setInfo(m_edit->getText());
					m_editShaderNode->set(m_edit->getText());
					m_edit->setVisible(false);
				}
				else if (event->getVirtualKey() == ui::VkEscape)
				{
					m_edit->setVisible(false);
				}
			}
		);
	}

	m_edit->setText(m_editShaderNode->get());
	m_edit->setRect(rcEdit);
	m_edit->setVisible(true);
	m_edit->selectAll();
	m_edit->setFocus();
}

void SwizzleNodeFacade::refreshEditorNode(
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

void SwizzleNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

}
