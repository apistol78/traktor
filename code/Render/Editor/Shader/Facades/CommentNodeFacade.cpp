/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/Facades/CommentNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/CommentNodeShape.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CommentNodeFacade", CommentNodeFacade, INodeFacade)

CommentNodeFacade::CommentNodeFacade()
{
	m_nodeShape = new ui::CommentNodeShape();
}

Ref< Node > CommentNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->createInstance());
}

Ref< ui::Node > CommentNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::Node > editorNode = graphControl->createNode(
		L"",
		L"",
		ui::UnitPoint(
			ui::Unit(shaderNode->getPosition().first),
			ui::Unit(shaderNode->getPosition().second)
		),
		m_nodeShape
	);
	editorNode->setComment(shaderNode->getComment());
	return editorNode;
}

void CommentNodeFacade::editShaderNode(
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
	m_editShaderNode = shaderNode;

	if (m_edit == nullptr)
	{
		m_edit = new ui::Edit();
		m_edit->create(graphControl);
		m_edit->addEventHandler< ui::FocusEvent >(
			[=, this](ui::FocusEvent* event)
			{
				if (m_edit->isVisible(false) && event->lostFocus())
				{
					m_editEditorNode->setComment(m_edit->getText());
					m_editShaderNode->setComment(m_edit->getText());
					m_edit->setVisible(false);
				}
			}
		);
		m_edit->addEventHandler< ui::KeyDownEvent >(
			[=, this](ui::KeyDownEvent* event)
			{
				if (event->getVirtualKey() == ui::VkReturn)
				{
					m_editEditorNode->setComment(m_edit->getText());
					m_editShaderNode->setComment(m_edit->getText());
					m_edit->setVisible(false);
				}
				else if (event->getVirtualKey() == ui::VkEscape)
				{
					m_edit->setVisible(false);
				}
			}
		);
	}

	m_edit->setText(shaderNode->getComment());
	m_edit->setRect(rcEdit);
	m_edit->setVisible(true);
	m_edit->selectAll();
	m_edit->setFocus();
}

void CommentNodeFacade::refreshEditorNode(
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

void CommentNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
}

}
