/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Facades/ParameterNodeFacade.h"

#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/InOutNodeShape.h"
#include "Ui/Graph/Node.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ParameterNodeFacade", ParameterNodeFacade, INodeFacade)

ParameterNodeFacade::ParameterNodeFacade()
{
	m_nodeShape = new ui::InOutNodeShape(ui::InOutNodeShape::StUniform);
}

Ref< Node > ParameterNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor)
{
	return new Parameter();
}

Ref< ui::Node > ParameterNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	std::wstring information;

	const Guid declarationId = mandatory_non_null_type_cast< Parameter* >(shaderNode)->getDeclarationId();
	if (declarationId.isNotNull())
	{
		Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(declarationId);
		information = (instance != nullptr) ? instance->getName() : declarationId.format();
	}
	else
		information = i18n::Text(L"SHADERGRAPH_NONE");

	Ref< ui::Node > editorNode = graphControl->createNode(
		L"",
		information,
		ui::UnitPoint(
			ui::Unit(shaderNode->getPosition().first),
			ui::Unit(shaderNode->getPosition().second)),
		m_nodeShape);

	for (int32_t j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			inputPin->getId(),
			!inputPin->isOptional(),
			false);
	}

	for (int32_t j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName(),
			outputPin->getId());
	}

	editorNode->setComment(shaderNode->getComment());

	return editorNode;
}

void ParameterNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
}

void ParameterNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	std::wstring information;

	const Guid declarationId = mandatory_non_null_type_cast< Parameter* >(shaderNode)->getDeclarationId();
	if (declarationId.isNotNull())
	{
		Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(declarationId);
		information = (instance != nullptr) ? instance->getName() : declarationId.format();
	}
	else
		information = i18n::Text(L"SHADERGRAPH_NONE");

	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(information);
}

void ParameterNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

}
