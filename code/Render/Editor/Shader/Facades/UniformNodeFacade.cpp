/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/UniformNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/InOutNodeShape.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformNodeFacade", UniformNodeFacade, INodeFacade)

UniformNodeFacade::UniformNodeFacade()
{
	m_nodeShape = new ui::InOutNodeShape(ui::InOutNodeShape::StUniform);
}

Ref< Node > UniformNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Uniform();
}

Ref< ui::Node > UniformNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	std::wstring information;

	const Guid declarationId = mandatory_non_null_type_cast< Uniform* >(shaderNode)->getDeclaration();
	if (declarationId.isNotNull())
	{
		Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(declarationId);
		information = (instance != nullptr) ? instance->getName() : declarationId.format();
	}
	else
		information = shaderNode->getInformation();

	Ref< ui::Node > editorNode = graphControl->createNode(
		L"",
		information,
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

void UniformNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
}

void UniformNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	std::wstring information;

	const Guid declarationId = mandatory_non_null_type_cast< Uniform* >(shaderNode)->getDeclaration();
	if (declarationId.isNotNull())
	{
		Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(declarationId);
		information = (instance != nullptr) ? instance->getName() : declarationId.format();
	}
	else
		information = shaderNode->getInformation();

	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(information);
}

void UniformNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

}
