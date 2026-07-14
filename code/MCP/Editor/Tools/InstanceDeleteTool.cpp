/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/InstanceDeleteTool.h"

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/Json.h"

namespace traktor::mcp
{
namespace
{

/*! Remove a single instance (checkout, remove, commit). */
bool removeInstance(db::Instance* instance)
{
	if (!instance->checkout())
		return false;
	if (!instance->remove() || !instance->commit())
	{
		instance->revert();
		return false;
	}
	return true;
}

/*! Recursively remove a group: child groups first, then child instances, then
 * the (now empty) group itself. Counts removed instances. */
bool removeGroupRecursive(db::Group* group, int32_t& outRemovedInstances)
{
	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);
	for (auto childGroup : childGroups)
		if (!removeGroupRecursive(childGroup, outRemovedInstances))
			return false;

	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);
	for (auto childInstance : childInstances)
	{
		if (!removeInstance(childInstance))
			return false;
		++outRemovedInstances;
	}

	return group->remove();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.InstanceDeleteTool", InstanceDeleteTool, IMcpTool)

InstanceDeleteTool::InstanceDeleteTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring InstanceDeleteTool::getName() const
{
	return L"delete_instance";
}

std::wstring InstanceDeleteTool::getDescription() const
{
	return L"Permanently delete an instance from the source database, identified by \"guid\" (preferred) or \"path\". If \"path\" resolves to a group instead of an instance, the group is deleted; pass \"recursive\"=true to also delete its contents (subgroups and instances), otherwise a non-empty group is left untouched. Intended for cleaning up temporary/throwaway instances. This is irreversible.";
}

Ref< Json > InstanceDeleteTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the instance to delete (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the instance, or of a group (alternative to guid).");

	Ref< Json > recursiveProperty = Json::createObject();
	recursiveProperty->setString(L"type", L"boolean");
	recursiveProperty->setString(L"description", L"When the path is a group, also delete its contents (default false).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"recursive", recursiveProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > InstanceDeleteTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	const std::wstring guidStr = (arguments && arguments->getMember(L"guid")) ? arguments->getMember(L"guid")->getString() : L"";
	const std::wstring path = (arguments && arguments->getMember(L"path")) ? arguments->getMember(L"path")->getString() : L"";
	const bool recursive = arguments && arguments->getMember(L"recursive") && arguments->getMember(L"recursive")->getBoolean();

	// Resolve an instance from guid (preferred) or path.
	Ref< db::Instance > instance;
	if (!guidStr.empty())
	{
		const Guid g(guidStr);
		if (!g.isValid())
		{
			outError = L"Invalid guid: " + guidStr;
			return nullptr;
		}
		instance = database->getInstance(g);
		if (!instance)
		{
			outError = L"No instance found for guid " + guidStr + L".";
			return nullptr;
		}
	}
	else if (!path.empty())
		instance = database->getInstance(path);
	else
	{
		outError = L"Missing \"guid\" or \"path\".";
		return nullptr;
	}

	if (instance)
	{
		const std::wstring deletedGuid = instance->getGuid().format();
		const std::wstring deletedPath = instance->getPath();
		if (!removeInstance(instance))
		{
			outError = L"Failed to delete instance \"" + deletedPath + L"\".";
			return nullptr;
		}
		Ref< Json > result = Json::createObject();
		result->setString(L"deleted", L"instance");
		result->setString(L"guid", deletedGuid);
		result->setString(L"path", deletedPath);
		return result;
	}

	// Not an instance; try interpreting the path as a group.
	Ref< db::Group > group = database->getGroup(path);
	if (!group)
	{
		outError = L"No instance or group found at \"" + path + L"\".";
		return nullptr;
	}

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);
	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);

	if ((!childGroups.empty() || !childInstances.empty()) && !recursive)
	{
		outError = L"Group \"" + path + L"\" is not empty; pass recursive=true to delete it and its contents.";
		return nullptr;
	}

	int32_t removedInstances = 0;
	if (!removeGroupRecursive(group, removedInstances))
	{
		outError = L"Failed to delete group \"" + path + L"\" (partially removed).";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"deleted", L"group");
	result->setString(L"path", path);
	result->set(L"removedInstances", Json::createNumber((int64_t)removedInstances));
	return result;
}

}
