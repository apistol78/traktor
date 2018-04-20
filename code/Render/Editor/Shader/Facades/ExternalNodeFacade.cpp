/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Render/Shader/External.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Facades/ExternalNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/ExternalNodeShape.h"
#include "Ui/Custom/Graph/PaintSettings.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ExternalNodeFacade", ExternalNodeFacade, INodeFacade)

ExternalNodeFacade::ExternalNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = new ui::custom::ExternalNodeShape(graphControl);
}

Ref< Node > ExternalNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	editor::TypeBrowseFilter filter(type_of< ShaderGraph >());
	Ref< db::Instance > fragmentInstance = editor->browseInstance(&filter);
	if (!fragmentInstance)
		return 0;

	Ref< ShaderGraph > fragmentGraph = fragmentInstance->getObject< ShaderGraph >();
	if (!fragmentGraph)
		return 0;

	return new External(
		fragmentInstance->getGuid(),
		fragmentGraph
	);
}

Ref< ui::custom::Node > ExternalNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	std::wstring title;

	Guid fragmentGuid = checked_type_cast< External* >(shaderNode)->getFragmentGuid();

	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(fragmentGuid);
	if (instance)
		title = instance->getName();
	else
		title = fragmentGuid.format();

	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		title,
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

void ExternalNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(
		checked_type_cast< External* >(shaderNode)->getFragmentGuid()
	);
	if (instance)
		editor->openEditor(instance);
}

void ExternalNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	External* external = checked_type_cast< External*, false >(shaderNode);
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(static_cast< External* >(shaderNode)->getFragmentGuid());
	editorNode->setTitle(instance ? instance->getName() : L"{ Null reference }");
	editorNode->setComment(shaderNode->getComment());
}

void ExternalNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
