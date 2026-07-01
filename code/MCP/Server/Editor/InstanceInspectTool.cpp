/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/InstanceInspectTool.h"

#include "Core/Reflection/Reflection.h"
#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/InstanceReflectionSupport.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Json.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.InstanceInspectTool", InstanceInspectTool, IMcpTool)

InstanceInspectTool::InstanceInspectTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring InstanceInspectTool::getName() const
{
	return L"inspect_instance";
}

std::wstring InstanceInspectTool::getDescription() const
{
	return L"Inspect any database instance via reflection. Returns the instance's type and a recursive list of its serialized members (name, kind=primitive|enum|compound|array|object, type and value). Works for any ISerializable - no type-specific knowledge needed. Member values feed directly into set_instance_member.";
}

Ref< Json > InstanceInspectTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the instance (alternative to guid).");

	Ref< Json > depthProperty = Json::createObject();
	depthProperty->setString(L"type", L"integer");
	depthProperty->setString(L"description", L"Maximum recursion depth into nested compounds/objects (default 8).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"maxDepth", depthProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > InstanceInspectTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	Ref< db::Instance > instance = resolveInstance(database, arguments, outError);
	if (!instance)
	{
		if (outError.empty())
			outError = L"No instance found for the given guid/path.";
		return nullptr;
	}

	int32_t maxDepth = c_defaultMaxDepth;
	if (arguments && arguments->getMember(L"maxDepth"))
		maxDepth = (int32_t)arguments->getMember(L"maxDepth")->getNumber(c_defaultMaxDepth);
	if (maxDepth < 0)
		maxDepth = 0;

	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		outError = L"Failed to read instance object.";
		return nullptr;
	}

	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
	{
		outError = L"Failed to create reflection of instance object.";
		return nullptr;
	}

	Ref< Json > members = Json::createArray();
	for (uint32_t i = 0; i < reflection->getMemberCount(); ++i)
		members->push(describeMember(reflection->getMember(i), maxDepth));

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setString(L"name", instance->getName());
	result->setString(L"type", type_name(object));
	result->set(L"members", members);
	return result;
}

}
