/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Node.h"
#include "Render/Editor/Shader/Facades/CommentNodeFacade.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/CommentNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CommentNodeFacade", CommentNodeFacade, INodeFacade)

CommentNodeFacade::CommentNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = new ui::custom::CommentNodeShape(graphControl);
}

Ref< Node > CommentNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->createInstance());
}

Ref< ui::custom::Node > CommentNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
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
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
}

void CommentNodeFacade::refreshEditorNode(
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

void CommentNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
}

	}
}
