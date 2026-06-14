/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Guid.h"
#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/InstanceCloneTool.h"
#include "MCP/Server/Editor/McpToolSupport.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.InstanceCloneTool", InstanceCloneTool, IMcpTool)

InstanceCloneTool::InstanceCloneTool(editor::IEditor* editor)
:	m_editor(editor)
{
}

std::wstring InstanceCloneTool::getName() const
{
	return L"clone_instance";
}

std::wstring InstanceCloneTool::getDescription() const
{
	return L"Deep-clone an existing database instance to a new path. The source is identified by \"guid\" or \"path\"; \"destPath\" is the new location. Returns the new instance's guid. Modify the copy afterwards with set_instance_member.";
}

Ref< Json > InstanceCloneTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the source instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the source instance (alternative to guid).");

	Ref< Json > destProperty = Json::createObject();
	destProperty->setString(L"type", L"string");
	destProperty->setString(L"description", L"Database path to create the clone at.");

	Ref< Json > newGuidProperty = Json::createObject();
	newGuidProperty->setString(L"type", L"string");
	newGuidProperty->setString(L"description", L"Optional explicit guid for the clone (otherwise one is generated).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"destPath", destProperty);
	properties->set(L"newGuid", newGuidProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"destPath"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > InstanceCloneTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	Ref< db::Instance > source = resolveInstance(database, arguments, outError);
	if (!source)
	{
		if (outError.empty())
			outError = L"No source instance found for the given guid/path.";
		return nullptr;
	}

	const std::wstring destPath = (arguments && arguments->getMember(L"destPath")) ? arguments->getMember(L"destPath")->getString() : L"";
	if (destPath.empty())
	{
		outError = L"Missing \"destPath\".";
		return nullptr;
	}
	if (database->getInstance(destPath))
	{
		outError = L"An instance already exists at \"" + destPath + L"\".";
		return nullptr;
	}

	Ref< ISerializable > sourceObject = source->getObject();
	if (!sourceObject)
	{
		outError = L"Failed to read source instance object.";
		return nullptr;
	}
	Ref< ISerializable > clone = DeepClone(sourceObject).create();
	if (!clone)
	{
		outError = L"Failed to deep-clone source instance.";
		return nullptr;
	}

	Guid guid;
	if (arguments->getMember(L"newGuid"))
	{
		const Guid g(arguments->getMember(L"newGuid")->getString());
		if (g.isValid())
			guid = g;
	}
	if (!guid.isValid())
		guid = Guid::create();

	Ref< db::Instance > instance = database->createInstance(destPath, db::CifReplaceExisting, &guid);
	if (!instance)
	{
		outError = L"Failed to create clone instance at \"" + destPath + L"\".";
		return nullptr;
	}
	if (!instance->setObject(clone) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the clone.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"sourceGuid", source->getGuid().format());
	result->setString(L"guid", guid.format());
	result->setString(L"path", destPath);
	result->setString(L"type", type_name(clone));
	result->setBoolean(L"committed", true);
	return result;
}

}
