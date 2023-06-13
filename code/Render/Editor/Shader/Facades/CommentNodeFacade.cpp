/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/Facades/CommentNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/CommentNodeShape.h"

namespace traktor
{
	namespace render
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
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
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
}
