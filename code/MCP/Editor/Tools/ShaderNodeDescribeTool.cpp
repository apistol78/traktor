/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/ShaderNodeDescribeTool.h"

#include "Core/Rtti/TypeInfo.h"
#include "MCP/Editor/NodeEmitterCapture.h"
#include "MCP/Editor/ShaderGraphToolSupport.h"
#include "MCP/Editor/Json.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ShaderNodeDescribeTool", ShaderNodeDescribeTool, IMcpTool)

std::wstring ShaderNodeDescribeTool::getName() const
{
	return L"describe_node";
}

std::wstring ShaderNodeDescribeTool::getDescription() const
{
	return L"Describe a single shader node type in depth: its input/output pins (with commutative input groups), whether it is a root/output node, and the GLSL emitter source that defines what it computes. Use the short type name from list_shader_node_types.";
}

Ref< Json > ShaderNodeDescribeTool::getInputSchema() const
{
	Ref< Json > typeProperty = Json::createObject();
	typeProperty->setString(L"type", L"string");
	typeProperty->setString(L"description", L"Short node type name (e.g. \"Add\", \"Dot\", \"Sampler\").");

	Ref< Json > properties = Json::createObject();
	properties->set(L"type", typeProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"type"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ShaderNodeDescribeTool::invoke(const Json* arguments, std::wstring& outError)
{
	const std::wstring type = (arguments && arguments->getMember(L"type")) ? arguments->getMember(L"type")->getString() : L"";
	if (type.empty())
	{
		outError = L"Missing \"type\".";
		return nullptr;
	}

	const std::wstring fullType = L"traktor.render." + type;
	Ref< render::Node > node = dynamic_type_cast< render::Node* >(TypeInfo::createInstance(fullType.c_str()));
	if (!node)
	{
		outError = L"Unknown node type: " + type;
		return nullptr;
	}

	// A single-node graph is enough for the (connection-independent) trait queries.
	Ref< render::ShaderGraph > graph = new render::ShaderGraph();
	graph->addNode(node);
	const render::INodeTraits* traits = render::INodeTraits::find(node);

	Ref< Json > result = Json::createObject();
	result->setString(L"type", type);
	if (isDeprecatedNode(type))
		result->setBoolean(L"deprecated", true);

	const std::wstring info = node->getInformation();
	if (!info.empty())
		result->setString(L"info", info);
	if (traits)
		result->setBoolean(L"isRoot", traits->isRoot(graph, node));

	Ref< Json > inputs = Json::createArray();
	for (int32_t p = 0; p < node->getInputPinCount(); ++p)
	{
		const render::InputPin* inputPin = node->getInputPin(p);
		Ref< Json > pin = Json::createObject();
		pin->setString(L"name", inputPin->getName());
		if (inputPin->isOptional())
			pin->setBoolean(L"optional", true);
		if (traits)
			pin->setNumber(L"group", traits->getInputPinGroup(graph, node, inputPin));
		inputs->push(pin);
	}
	result->set(L"inputs", inputs);

	Ref< Json > outputs = Json::createArray();
	for (int32_t p = 0; p < node->getOutputPinCount(); ++p)
		outputs->push(Json::createString(node->getOutputPin(p)->getName()));
	result->set(L"outputs", outputs);

	const auto& summary = nodeEmitterSummary();
	const auto summaryIt = summary.find(type);
	if (summaryIt != summary.end())
		result->setString(L"glsl", summaryIt->second);

	const auto& capture = nodeEmitterCapture();
	const auto it = capture.find(type);
	if (it != capture.end())
		result->setString(L"emitterSource", it->second);
	else
		result->setString(L"emitterNote", L"No GLSL emitter for this node; it is resolved away (e.g. fragments/ports) or handled by the editor/permutation stage.");

	return result;
}

}
