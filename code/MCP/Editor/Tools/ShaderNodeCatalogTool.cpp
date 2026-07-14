/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/ShaderNodeCatalogTool.h"

#include "Core/Misc/String.h"
#include "Core/Rtti/TypeInfo.h"
#include "MCP/Editor/NodeEmitterCapture.h"
#include "MCP/Editor/ShaderGraphToolSupport.h"
#include "MCP/Editor/Json.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ShaderNodeCatalogTool", ShaderNodeCatalogTool, IMcpTool)

std::wstring ShaderNodeCatalogTool::getName() const
{
	return L"list_shader_node_types";
}

std::wstring ShaderNodeCatalogTool::getDescription() const
{
	return L"List the available shader graph node types and their input/output pin names. Use this to discover which nodes can be used to build shader graphs. Optionally filter by a substring of the type name. Deprecated nodes are excluded by default and should not be used in new graphs.";
}

Ref< Json > ShaderNodeCatalogTool::getInputSchema() const
{
	Ref< Json > filterProperty = Json::createObject();
	filterProperty->setString(L"type", L"string");
	filterProperty->setString(L"description", L"Only include node types whose name contains this substring (case-insensitive).");

	Ref< Json > includeDeprecatedProperty = Json::createObject();
	includeDeprecatedProperty->setString(L"type", L"boolean");
	includeDeprecatedProperty->setString(L"description", L"Include deprecated node types (default false). When included they are flagged with \"deprecated\": true.");

	Ref< Json > properties = Json::createObject();
	properties->set(L"filter", filterProperty);
	properties->set(L"includeDeprecated", includeDeprecatedProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > ShaderNodeCatalogTool::invoke(const Json* arguments, std::wstring& outError)
{
	std::wstring filter;
	if (arguments && arguments->getMember(L"filter"))
		filter = toLower(arguments->getMember(L"filter")->getString());
	const bool includeDeprecated = arguments && arguments->getMember(L"includeDeprecated") && arguments->getMember(L"includeDeprecated")->getBoolean();

	Ref< Json > list = Json::createArray();
	int32_t count = 0;
	for (const TypeInfo* type : type_of< render::Node >().findAllOf(false))
	{
		if (!type->isInstantiable())
			continue;

		Ref< render::Node > node = dynamic_type_cast< render::Node* >(type->createInstance());
		if (!node)
			continue;

		const std::wstring name = shortTypeName(type->getName());
		if (!filter.empty() && toLower(name).find(filter) == std::wstring::npos)
			continue;

		const bool deprecated = isDeprecatedNode(name);
		if (deprecated && !includeDeprecated)
			continue;

		Ref< Json > entry = Json::createObject();
		entry->setString(L"type", name);
		if (deprecated)
			entry->setBoolean(L"deprecated", true);

		Ref< Json > inputs = Json::createArray();
		for (int32_t p = 0; p < node->getInputPinCount(); ++p)
		{
			const render::InputPin* inputPin = node->getInputPin(p);
			Ref< Json > pin = Json::createObject();
			pin->setString(L"name", inputPin->getName());
			if (inputPin->isOptional())
				pin->setBoolean(L"optional", true);
			inputs->push(pin);
		}
		entry->set(L"inputs", inputs);

		Ref< Json > outputs = Json::createArray();
		for (int32_t p = 0; p < node->getOutputPinCount(); ++p)
			outputs->push(Json::createString(node->getOutputPin(p)->getName()));
		entry->set(L"outputs", outputs);

		// Quick semantic hint: the one-line GLSL expression, when available.
		const auto summaryIt = nodeEmitterSummary().find(name);
		if (summaryIt != nodeEmitterSummary().end())
			entry->setString(L"glsl", summaryIt->second);

		list->push(entry);
		++count;
	}

	Ref< Json > result = Json::createObject();
	result->set(L"nodeTypes", list);
	result->setNumber(L"count", count);
	return result;
}

}
