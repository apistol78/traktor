/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/InstanceCreateTool.h"

#include "Core/Guid.h"
#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/Json.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.InstanceCreateTool", InstanceCreateTool, IMcpTool)

InstanceCreateTool::InstanceCreateTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring InstanceCreateTool::getName() const
{
	return L"create_instance";
}

std::wstring InstanceCreateTool::getDescription() const
{
	return L"Create a new, default-constructed database instance of a serializable type. \"type\" is the fully qualified type name (e.g. \"traktor.mesh.MeshAsset\"); \"path\" is where to create it. Populate fields afterwards with set_instance_member. Fails if the path already exists.";
}

Ref< Json > InstanceCreateTool::getInputSchema() const
{
	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path to create the instance at (e.g. \"Entities/MyAsset\").");

	Ref< Json > typeProperty = Json::createObject();
	typeProperty->setString(L"type", L"string");
	typeProperty->setString(L"description", L"Fully qualified serializable type name (e.g. \"traktor.mesh.MeshAsset\").");

	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Optional explicit guid for the new instance (otherwise one is generated).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"path", pathProperty);
	properties->set(L"type", typeProperty);
	properties->set(L"guid", guidProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"path"));
	required->push(Json::createString(L"type"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > InstanceCreateTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	const std::wstring path = (arguments && arguments->getMember(L"path")) ? arguments->getMember(L"path")->getString() : L"";
	const std::wstring typeName = (arguments && arguments->getMember(L"type")) ? arguments->getMember(L"type")->getString() : L"";
	if (path.empty() || typeName.empty())
	{
		outError = L"Missing \"path\" and/or \"type\".";
		return nullptr;
	}

	const TypeInfo* type = TypeInfo::find(typeName.c_str());
	if (!type)
	{
		outError = L"Unknown type: " + typeName + L".";
		return nullptr;
	}
	if (!type->isInstantiable())
	{
		outError = L"Type is not instantiable: " + typeName + L".";
		return nullptr;
	}

	Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(type->createInstance());
	if (!object)
	{
		outError = L"Type is not a serializable instance type: " + typeName + L".";
		return nullptr;
	}

	if (database->getInstance(path))
	{
		outError = L"An instance already exists at \"" + path + L"\".";
		return nullptr;
	}

	Guid guid;
	if (arguments->getMember(L"guid"))
	{
		const Guid g(arguments->getMember(L"guid")->getString());
		if (g.isValid())
			guid = g;
	}
	if (!guid.isValid())
		guid = Guid::create();

	Ref< db::Instance > instance = database->createInstance(path, db::CifReplaceExisting, &guid);
	if (!instance)
	{
		outError = L"Failed to create instance at \"" + path + L"\".";
		return nullptr;
	}
	if (!instance->setObject(object) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the new instance.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", guid.format());
	result->setString(L"path", path);
	result->setString(L"type", typeName);
	result->setBoolean(L"committed", true);
	return result;
}

}
