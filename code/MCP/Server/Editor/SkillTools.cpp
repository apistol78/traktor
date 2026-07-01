/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/SkillTools.h"

#include "Core/Guid.h"
#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Editor/SkillAsset.h"
#include "MCP/Server/Json.h"

namespace traktor::mcp
{
namespace
{

std::wstring argStr(const Json* arguments, const wchar_t* key, const std::wstring& def = L"")
{
	if (!arguments)
		return def;
	const Json* m = arguments->getMember(key);
	return (m && m->isString()) ? m->getString() : def;
}

bool argBool(const Json* arguments, const wchar_t* key, bool def)
{
	if (!arguments)
		return def;
	const Json* m = arguments->getMember(key);
	return m ? m->getBoolean(def) : def;
}

void findSkillInstances(db::Database* database, RefArray< db::Instance >& outInstances)
{
	Ref< db::Group > rootGroup = database->getRootGroup();
	if (!rootGroup)
		return;
	db::recursiveFindChildInstances(
		rootGroup,
		[](db::Instance* instance) {
		const TypeInfo* t = instance->getPrimaryType();
		return t != nullptr && is_type_of(type_of< SkillAsset >(), *t);
	},
		outInstances);
}

Ref< db::Instance > findSkill(db::Database* database, const std::wstring& nameOrGuid, Ref< SkillAsset >& outSkill)
{
	RefArray< db::Instance > instances;
	findSkillInstances(database, instances);
	for (auto instance : instances)
	{
		Ref< SkillAsset > skill = instance->getObject< SkillAsset >();
		if (!skill)
			continue;
		if (skill->m_name == nameOrGuid || instance->getName() == nameOrGuid || instance->getGuid().format() == nameOrGuid)
		{
			outSkill = skill;
			return instance;
		}
	}
	return nullptr;
}

std::wstring substitute(const std::wstring& body, const Json* arguments, const RefArray< SkillParameter >& parameters)
{
	std::wstring out = body;
	for (auto p : parameters)
	{
		if (!p || p->m_name.empty())
			continue;
		std::wstring value = p->m_defaultValue;
		if (arguments)
		{
			const Json* a = arguments->getMember(p->m_name);
			if (a && a->isString())
				value = a->getString();
			else if (a && !a->isNull())
				value = a->toString();
		}
		const std::wstring token = L"{{" + p->m_name + L"}}";
		std::wstring::size_type pos = 0;
		while ((pos = out.find(token, pos)) != std::wstring::npos)
		{
			out.replace(pos, token.length(), value);
			pos += value.length();
		}
	}
	return out;
}

Ref< Json > skillToJson(db::Instance* instance, SkillAsset* skill, bool includeBody)
{
	Ref< Json > entry = Json::createObject();
	entry->setString(L"name", skill->m_name);
	entry->setString(L"description", skill->m_description);
	entry->setString(L"whenToUse", skill->m_whenToUse);
	entry->setBoolean(L"published", skill->m_published);
	entry->setString(L"path", instance->getPath());
	entry->setString(L"guid", instance->getGuid().format());
	if (!skill->m_engineVersion.empty())
		entry->setString(L"engineVersion", skill->m_engineVersion);
	if (includeBody)
		entry->setString(L"body", skill->m_body);

	Ref< Json > params = Json::createArray();
	for (auto p : skill->m_parameters)
	{
		if (!p)
			continue;
		Ref< Json > pj = Json::createObject();
		pj->setString(L"name", p->m_name);
		pj->setString(L"description", p->m_description);
		pj->setString(L"defaultValue", p->m_defaultValue);
		params->push(pj);
	}
	entry->set(L"parameters", params);
	return entry;
}

Ref< Json > stringProperty(const std::wstring& description)
{
	Ref< Json > p = Json::createObject();
	p->setString(L"type", L"string");
	p->setString(L"description", description);
	return p;
}

Ref< Json > boolProperty(const std::wstring& description)
{
	Ref< Json > p = Json::createObject();
	p->setString(L"type", L"boolean");
	p->setString(L"description", description);
	return p;
}

}

// ---------------------------------------------------------------------------

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.SkillCreateTool", SkillCreateTool, IMcpTool)

SkillCreateTool::SkillCreateTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring SkillCreateTool::getName() const
{
	return L"create_skill";
}

std::wstring SkillCreateTool::getDescription() const
{
	return L"Create or overwrite an AI skill (traktor.mcp.SkillAsset) stored as a database instance. A skill captures a reusable procedure as a markdown \"body\" plus typed \"parameters\"; {{parameter}} tokens in the body are substituted from the arguments when the skill is invoked as a prompt. Provide \"path\", \"name\" (the prompt/slash-command id), and \"body\"; optionally \"description\", \"whenToUse\", \"published\", \"engineVersion\", and \"parameters\" (array of {name, description, defaultValue}). Overwrites any instance already at \"path\". Publish it (here or with publish_skill) to expose it over prompts/list.";
}

Ref< Json > SkillCreateTool::getInputSchema() const
{
	Ref< Json > paramItem = Json::createObject();
	paramItem->setString(L"type", L"object");
	Ref< Json > paramProps = Json::createObject();
	paramProps->set(L"name", stringProperty(L"Parameter name; referenced in the body as {{name}}."));
	paramProps->set(L"description", stringProperty(L"What the parameter controls."));
	paramProps->set(L"defaultValue", stringProperty(L"Value used when the argument is omitted."));
	paramItem->set(L"properties", paramProps);

	Ref< Json > parametersProperty = Json::createObject();
	parametersProperty->setString(L"type", L"array");
	parametersProperty->setString(L"description", L"Typed parameters substituted into the body.");
	parametersProperty->set(L"items", paramItem);

	Ref< Json > properties = Json::createObject();
	properties->set(L"path", stringProperty(L"Database path to store the skill at (e.g. \"Skills/Create Animated Character\")."));
	properties->set(L"name", stringProperty(L"Skill / prompt name (the slash-command id)."));
	properties->set(L"description", stringProperty(L"One-line summary used for matching."));
	properties->set(L"whenToUse", stringProperty(L"Trigger conditions describing when to apply the skill."));
	properties->set(L"body", stringProperty(L"Markdown procedure; use {{parameter}} tokens for substitution."));
	properties->set(L"engineVersion", stringProperty(L"Optional engine version the skill targets."));
	properties->set(L"published", boolProperty(L"Whether to expose the skill as a prompt immediately (default false)."));
	properties->set(L"parameters", parametersProperty);
	properties->set(L"guid", stringProperty(L"Optional explicit guid for the instance."));

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"path"));
	required->push(Json::createString(L"name"));
	required->push(Json::createString(L"body"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > SkillCreateTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	// Recover any structured arguments a client delivered as JSON-encoded strings.
	Ref< Json > coerced = coerceStructuredArguments(arguments);
	arguments = coerced;

	const std::wstring path = argStr(arguments, L"path");
	const std::wstring name = argStr(arguments, L"name");
	const std::wstring body = argStr(arguments, L"body");
	if (path.empty() || name.empty() || body.empty())
	{
		outError = L"Missing \"path\", \"name\" and/or \"body\".";
		return nullptr;
	}

	Ref< SkillAsset > skill = new SkillAsset();
	skill->m_name = name;
	skill->m_description = argStr(arguments, L"description");
	skill->m_whenToUse = argStr(arguments, L"whenToUse");
	skill->m_body = body;
	skill->m_engineVersion = argStr(arguments, L"engineVersion");
	skill->m_published = argBool(arguments, L"published", false);

	if (arguments)
	{
		const Json* ps = arguments->getMember(L"parameters");
		if (ps && ps->isArray())
		{
			for (uint32_t i = 0; i < ps->size(); ++i)
			{
				const Json* p = ps->at(i);
				if (!p || !p->isObject())
					continue;
				Ref< SkillParameter > sp = new SkillParameter();
				sp->m_name = (p->getMember(L"name") ? p->getMember(L"name")->getString() : L"");
				sp->m_description = (p->getMember(L"description") ? p->getMember(L"description")->getString() : L"");
				sp->m_defaultValue = (p->getMember(L"defaultValue") ? p->getMember(L"defaultValue")->getString() : L"");
				if (!sp->m_name.empty())
					skill->m_parameters.push_back(sp);
			}
		}
	}

	Guid guid;
	if (arguments && arguments->getMember(L"guid"))
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
	if (!instance->setObject(skill) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the skill instance.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setString(L"name", skill->m_name);
	result->setBoolean(L"published", skill->m_published);
	result->setBoolean(L"committed", true);
	return result;
}

// ---------------------------------------------------------------------------

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.SkillListTool", SkillListTool, IMcpTool)

SkillListTool::SkillListTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring SkillListTool::getName() const
{
	return L"list_skills";
}

std::wstring SkillListTool::getDescription() const
{
	return L"List the AI skills stored in the database (name, description, whenToUse, published flag, path, guid, parameters). Pass \"publishedOnly\": true for only published skills. Use this to discover an existing skill before authoring, then get_skill to read the full body.";
}

Ref< Json > SkillListTool::getInputSchema() const
{
	Ref< Json > properties = Json::createObject();
	properties->set(L"publishedOnly", boolProperty(L"List only published skills (default false)."));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	return schema;
}

Ref< Json > SkillListTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	const bool publishedOnly = argBool(arguments, L"publishedOnly", false);

	RefArray< db::Instance > instances;
	findSkillInstances(database, instances);

	Ref< Json > list = Json::createArray();
	int32_t count = 0;
	for (auto instance : instances)
	{
		Ref< SkillAsset > skill = instance->getObject< SkillAsset >();
		if (!skill)
			continue;
		if (publishedOnly && !skill->m_published)
			continue;
		list->push(skillToJson(instance, skill, false));
		++count;
	}

	Ref< Json > result = Json::createObject();
	result->set(L"skills", list);
	result->setNumber(L"count", count);
	return result;
}

// ---------------------------------------------------------------------------

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.SkillGetTool", SkillGetTool, IMcpTool)

SkillGetTool::SkillGetTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring SkillGetTool::getName() const
{
	return L"get_skill";
}

std::wstring SkillGetTool::getDescription() const
{
	return L"Fetch a single skill's full markdown body, parameters, and metadata. Identify it by \"name\" (the skill name) or \"guid\". Follow the returned procedure to perform the task.";
}

Ref< Json > SkillGetTool::getInputSchema() const
{
	Ref< Json > properties = Json::createObject();
	properties->set(L"name", stringProperty(L"Skill name to fetch."));
	properties->set(L"guid", stringProperty(L"Skill instance guid (alternative to name)."));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	return schema;
}

Ref< Json > SkillGetTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	std::wstring key = argStr(arguments, L"name");
	if (key.empty())
		key = argStr(arguments, L"guid");
	if (key.empty())
	{
		outError = L"Provide \"name\" or \"guid\".";
		return nullptr;
	}

	Ref< SkillAsset > skill;
	Ref< db::Instance > instance = findSkill(database, key, skill);
	if (!instance)
	{
		outError = L"No skill found for \"" + key + L"\".";
		return nullptr;
	}

	return skillToJson(instance, skill, true);
}

// ---------------------------------------------------------------------------

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.SkillPublishTool", SkillPublishTool, IMcpTool)

SkillPublishTool::SkillPublishTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring SkillPublishTool::getName() const
{
	return L"publish_skill";
}

std::wstring SkillPublishTool::getDescription() const
{
	return L"Publish or unpublish a skill. Identify by \"name\" or \"guid\". A published skill is exposed over MCP prompts/list and prompts/get (appearing as a slash command in the client). Pass \"published\": false to retract (defaults to true).";
}

Ref< Json > SkillPublishTool::getInputSchema() const
{
	Ref< Json > properties = Json::createObject();
	properties->set(L"name", stringProperty(L"Skill name to publish."));
	properties->set(L"guid", stringProperty(L"Skill instance guid (alternative to name)."));
	properties->set(L"published", boolProperty(L"Target published state (default true)."));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	return schema;
}

Ref< Json > SkillPublishTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	std::wstring key = argStr(arguments, L"name");
	if (key.empty())
		key = argStr(arguments, L"guid");
	if (key.empty())
	{
		outError = L"Provide \"name\" or \"guid\".";
		return nullptr;
	}

	Ref< SkillAsset > skill;
	Ref< db::Instance > instance = findSkill(database, key, skill);
	if (!instance)
	{
		outError = L"No skill found for \"" + key + L"\".";
		return nullptr;
	}

	skill->m_published = argBool(arguments, L"published", true);

	if (!instance->checkout())
	{
		outError = L"Failed to checkout skill for writing.";
		return nullptr;
	}
	if (!instance->setObject(skill) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the skill.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"name", skill->m_name);
	result->setString(L"path", instance->getPath());
	result->setString(L"guid", instance->getGuid().format());
	result->setBoolean(L"published", skill->m_published);
	return result;
}

// ---------------------------------------------------------------------------

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.SkillPromptProvider", SkillPromptProvider, IMcpPromptProvider)

SkillPromptProvider::SkillPromptProvider(editor::IEditor* editor)
	: m_editor(editor)
{
}

void SkillPromptProvider::listPrompts(Json* outPrompts)
{
	if (!outPrompts)
		return;

	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
		return;

	RefArray< db::Instance > instances;
	findSkillInstances(database, instances);
	for (auto instance : instances)
	{
		Ref< SkillAsset > skill = instance->getObject< SkillAsset >();
		if (!skill || !skill->m_published || skill->m_name.empty())
			continue;

		Ref< Json > entry = Json::createObject();
		entry->setString(L"name", skill->m_name);

		std::wstring description = skill->m_description;
		if (!skill->m_whenToUse.empty())
			description += (description.empty() ? L"" : L" ") + (L"(Use when: " + skill->m_whenToUse + L")");
		entry->setString(L"description", description);

		Ref< Json > args = Json::createArray();
		for (auto p : skill->m_parameters)
		{
			if (!p || p->m_name.empty())
				continue;
			Ref< Json > a = Json::createObject();
			a->setString(L"name", p->m_name);
			a->setString(L"description", p->m_description);
			a->setBoolean(L"required", false);
			args->push(a);
		}
		entry->set(L"arguments", args);

		outPrompts->push(entry);
	}
}

Ref< Json > SkillPromptProvider::getPrompt(const std::wstring& name, const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
		return nullptr;

	RefArray< db::Instance > instances;
	findSkillInstances(database, instances);
	for (auto instance : instances)
	{
		Ref< SkillAsset > skill = instance->getObject< SkillAsset >();
		if (!skill || !skill->m_published || skill->m_name != name)
			continue;

		const std::wstring text = substitute(skill->m_body, arguments, skill->m_parameters);

		Ref< Json > content = Json::createObject();
		content->setString(L"type", L"text");
		content->setString(L"text", text);

		Ref< Json > message = Json::createObject();
		message->setString(L"role", L"user");
		message->set(L"content", content);

		Ref< Json > messages = Json::createArray();
		messages->push(message);

		Ref< Json > result = Json::createObject();
		result->setString(L"description", skill->m_description);
		result->set(L"messages", messages);
		return result;
	}

	// Not owned by this provider.
	return nullptr;
}

}
