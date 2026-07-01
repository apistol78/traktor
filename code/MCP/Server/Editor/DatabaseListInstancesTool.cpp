/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/DatabaseListInstancesTool.h"

#include "Core/Guid.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Json.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.DatabaseListInstancesTool", DatabaseListInstancesTool, IMcpTool)

DatabaseListInstancesTool::DatabaseListInstancesTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring DatabaseListInstancesTool::getName() const
{
	return L"list_instances";
}

std::wstring DatabaseListInstancesTool::getDescription() const
{
	return L"List asset instances in the editor's source database. Optionally filter by primary type name (substring match) and/or group path prefix. Returns each instance's name, guid, primary type and path.";
}

Ref< Json > DatabaseListInstancesTool::getInputSchema() const
{
	Ref< Json > typeNameProperty = Json::createObject();
	typeNameProperty->setString(L"type", L"string");
	typeNameProperty->setString(L"description", L"Only include instances whose primary type name contains this substring (e.g. \"ShaderGraph\").");

	Ref< Json > groupPathProperty = Json::createObject();
	groupPathProperty->setString(L"type", L"string");
	groupPathProperty->setString(L"description", L"Only include instances whose database path begins with this group path.");

	Ref< Json > limitProperty = Json::createObject();
	limitProperty->setString(L"type", L"integer");
	limitProperty->setString(L"description", L"Maximum number of instances to return (default 200).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"typeName", typeNameProperty);
	properties->set(L"groupPath", groupPathProperty);
	properties->set(L"limit", limitProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > DatabaseListInstancesTool::invoke(const Json* arguments, std::wstring& outError)
{
	// NOTE: this runs on the MCP server thread, not the editor thread. The
	// database guards individual operations internally, but a future revision
	// should marshal tool calls onto the editor thread to be fully safe against
	// concurrent workspace mutations.
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	std::wstring typeFilter;
	std::wstring groupFilter;
	int32_t limit = 200;
	if (arguments)
	{
		if (Json* value = arguments->getMember(L"typeName"))
			typeFilter = value->getString();
		if (Json* value = arguments->getMember(L"groupPath"))
			groupFilter = value->getString();
		if (Json* value = arguments->getMember(L"limit"))
			limit = (int32_t)value->getNumber(200);
	}
	if (limit <= 0)
		limit = 200;

	Ref< db::Group > rootGroup = database->getRootGroup();
	if (!rootGroup)
	{
		outError = L"Source database has no root group.";
		return nullptr;
	}

	RefArray< db::Instance > instances;
	db::recursiveFindChildInstances(rootGroup, [](db::Instance*) {
		return true;
	}, instances);

	Ref< Json > list = Json::createArray();
	int32_t count = 0;
	bool truncated = false;
	for (auto instance : instances)
	{
		const std::wstring typeName = instance->getPrimaryTypeName();
		const std::wstring path = instance->getPath();

		if (!typeFilter.empty() && typeName.find(typeFilter) == std::wstring::npos)
			continue;
		if (!groupFilter.empty() && path.find(groupFilter) != 0)
			continue;

		if (count >= limit)
		{
			truncated = true;
			break;
		}

		Ref< Json > entry = Json::createObject();
		entry->setString(L"name", instance->getName());
		entry->setString(L"guid", instance->getGuid().format());
		entry->setString(L"type", typeName);
		entry->setString(L"path", path);
		list->push(entry);
		++count;
	}

	Ref< Json > result = Json::createObject();
	result->set(L"instances", list);
	result->setNumber(L"count", count);
	result->setBoolean(L"truncated", truncated);
	return result;
}

}
