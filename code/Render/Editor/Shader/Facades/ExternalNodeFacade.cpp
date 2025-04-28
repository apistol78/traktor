/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Facades/ExternalNodeFacade.h"

#include "Core/Io/StringOutputStream.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Ui/Application.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/PaintSettings.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ExternalNodeFacade", ExternalNodeFacade, INodeFacade)

ExternalNodeFacade::ExternalNodeFacade()
{
	m_nodeShape = new ui::DefaultNodeShape(ui::DefaultNodeShape::StExternal);
}

Ref< Node > ExternalNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor)
{
	editor::TypeBrowseFilter filter(type_of< ShaderGraph >());
	Ref< db::Instance > fragmentInstance = editor->browseInstance(&filter);
	if (!fragmentInstance)
		return nullptr;

	Ref< ShaderGraph > fragmentGraph = fragmentInstance->getObject< ShaderGraph >();
	if (!fragmentGraph)
		return nullptr;

	return new External(
		fragmentInstance->getGuid(),
		fragmentGraph);
}

Ref< ui::Node > ExternalNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	External* externalNode = mandatory_non_null_type_cast< External* >(shaderNode);
	std::wstring title;

	const Guid fragmentGuid = externalNode->getFragmentGuid();

	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(fragmentGuid);
	if (instance)
		title = instance->getName();
	else
		title = fragmentGuid.format();

	Ref< ui::Node > editorNode = graphControl->createNode(
		title,
		L"",
		ui::UnitPoint(
			ui::Unit(shaderNode->getPosition().first),
			ui::Unit(shaderNode->getPosition().second)),
		m_nodeShape);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			inputPin->getId(),
			!inputPin->isOptional(),
			false);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName(),
			outputPin->getId());
	}

	editorNode->setComment(shaderNode->getComment());

	return editorNode;
}

void ExternalNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(
		checked_type_cast< External* >(shaderNode)->getFragmentGuid());
	if (instance)
		editor->openEditor(instance);
}

void ExternalNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	External* external = checked_type_cast< External*, false >(shaderNode);
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(static_cast< External* >(shaderNode)->getFragmentGuid());
	editorNode->setTitle(instance ? instance->getName() : L"{ Null reference }");
	editorNode->setComment(shaderNode->getComment());
}

void ExternalNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

}
