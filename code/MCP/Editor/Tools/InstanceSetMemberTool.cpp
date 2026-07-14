/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/InstanceSetMemberTool.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/InstanceReflectionSupport.h"
#include "MCP/Editor/McpToolSupport.h"
#include "MCP/Editor/Json.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.InstanceSetMemberTool", InstanceSetMemberTool, IMcpTool)

InstanceSetMemberTool::InstanceSetMemberTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring InstanceSetMemberTool::getName() const
{
	return L"set_instance_member";
}

std::wstring InstanceSetMemberTool::getDescription() const
{
	return L"Set a member value on any database instance via reflection. \"member\" is a dotted/indexed path that may descend through nested object references and arrays (e.g. \"grounded\", \"components[0].entityData[1].transform.translation\"); \"value\" is the new value. For primitive/enum leaves: a bool/number/string, enum key, or number array (vector/color/quaternion). For object members: { \"$clone\": \"<guid|path>\" } or { \"$type\": \"<typename>\", \"set\": { member: value, ... } }, or null. For array members: a JSON array of element specs (object specs, or guid strings for resource::Id sets); replaces the array, or pass \"append\":true to add the element(s) to the existing array (existing elements preserved). For map (SmallMap) members pass a JSON object { key: valueSpec, ... } to set entries (value is an object spec, guid string, or null). To grow a typed vector (struct/enum/primitive elements) pass { \"$grow\": N }, then set each element by index path (e.g. \"entries[0].mul\"). The change is applied and committed. Use inspect_instance first to discover member names and types.";
}

Ref< Json > InstanceSetMemberTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the instance (alternative to guid).");

	Ref< Json > memberProperty = Json::createObject();
	memberProperty->setString(L"type", L"string");
	memberProperty->setString(L"description", L"Dotted/indexed path to the member to set (e.g. \"grounded\", \"a.b[2].c\").");

	// "value" is polymorphic (bool/number/string/array/object/null). It MUST
	// declare a type: a typeless property is delivered by some MCP clients as a
	// raw string, which silently breaks numeric, boolean and array members.
	Ref< Json > valueType = Json::createArray();
	valueType->push(Json::createString(L"boolean"));
	valueType->push(Json::createString(L"integer"));
	valueType->push(Json::createString(L"number"));
	valueType->push(Json::createString(L"string"));
	valueType->push(Json::createString(L"array"));
	valueType->push(Json::createString(L"object"));
	valueType->push(Json::createString(L"null"));

	Ref< Json > valueProperty = Json::createObject();
	valueProperty->set(L"type", valueType);
	valueProperty->setString(L"description", L"New value: bool, number, string (also enum key, guid or path), or array of numbers (vector/color/quaternion).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"member", memberProperty);
	Ref< Json > appendProperty = Json::createObject();
	appendProperty->setString(L"type", L"boolean");
	appendProperty->setString(L"description", L"For array members, add the given element(s) to the existing array instead of replacing it (default false).");

	properties->set(L"value", valueProperty);
	properties->set(L"append", appendProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"member"));
	required->push(Json::createString(L"value"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > InstanceSetMemberTool::invoke(const Json* arguments, std::wstring& outError)
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

	const std::wstring memberPath = (arguments && arguments->getMember(L"member")) ? arguments->getMember(L"member")->getString() : L"";
	if (memberPath.empty())
	{
		outError = L"Missing \"member\" (the member path).";
		return nullptr;
	}
	const Json* value = arguments ? arguments->getMember(L"value") : nullptr;
	if (!value)
	{
		outError = L"Missing \"value\".";
		return nullptr;
	}

	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		outError = L"Failed to read instance object.";
		return nullptr;
	}

	AlignedVector< PathStep > steps;
	if (!tokenizePath(memberPath, steps, outError))
		return nullptr;

	const bool append = arguments && arguments->getMember(L"append") && arguments->getMember(L"append")->getBoolean();
	if (!setMemberThroughPath(database, object, steps, 0, value, outError, append))
		return nullptr;

	if (!instance->checkout())
	{
		outError = L"Failed to checkout instance for writing.";
		return nullptr;
	}
	if (!instance->setObject(object) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the updated instance.";
		return nullptr;
	}

	// Re-read so the echoed value reflects the committed, round-tripped state.
	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setString(L"type", type_name(object));
	result->setString(L"member", memberPath);
	result->setBoolean(L"committed", true);

	Ref< ISerializable > updated = instance->getObject();
	Ref< Json > current = updated ? describeMemberAtPath(updated, steps, 0) : nullptr;
	if (current)
		result->set(L"current", current);
	return result;
}

}
