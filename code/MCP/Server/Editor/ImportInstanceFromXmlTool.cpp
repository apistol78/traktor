/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/MemoryStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/TString.h"
#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Xml/XmlDeserializer.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/ImportInstanceFromXmlTool.h"
#include "MCP/Server/Editor/McpToolSupport.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ImportInstanceFromXmlTool", ImportInstanceFromXmlTool, IMcpTool)

ImportInstanceFromXmlTool::ImportInstanceFromXmlTool(editor::IEditor* editor)
:	m_editor(editor)
{
}

std::wstring ImportInstanceFromXmlTool::getName() const
{
	return L"import_instance_from_xml";
}

std::wstring ImportInstanceFromXmlTool::getDescription() const
{
	return L"Replace an existing instance's object by deserializing a provided XML document (the .xdi object form, i.e. <object type=\"...\">...</object>). Unlike set_instance_member, the XML deserializer resolves ref= ids into shared object references, so this can author assets whose editor representation relies on object identity (e.g. input mappings, shader graphs). Identify the instance by \"guid\" (preferred) or \"path\"; \"xml\" is the full object document. The change is committed. Get the current XML via inspect/source first and edit surgically.";
}

Ref< Json > ImportInstanceFromXmlTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the instance to replace (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the instance (alternative to guid).");

	Ref< Json > xmlProperty = Json::createObject();
	xmlProperty->setString(L"type", L"string");
	xmlProperty->setString(L"description", L"Full XML object document (<object type=\"...\">...</object>) to deserialize and commit.");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"xml", xmlProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"xml"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ImportInstanceFromXmlTool::invoke(const Json* arguments, std::wstring& outError)
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

	const std::wstring xml = (arguments && arguments->getMember(L"xml")) ? arguments->getMember(L"xml")->getString() : L"";
	if (xml.empty())
	{
		outError = L"Missing \"xml\".";
		return nullptr;
	}

	// Deserialize the supplied XML (UTF-8) into an object; ref= ids become shared references.
	const std::string xml8 = wstombs(Utf8Encoding(), xml);
	MemoryStream ms((void*)xml8.c_str(), (int64_t)xml8.size(), true, false);
	Ref< ISerializable > object = xml::XmlDeserializer(&ms, instance->getPath()).readObject();
	if (!object)
	{
		outError = L"Failed to parse the supplied XML into an object.";
		return nullptr;
	}

	if (!instance->checkout())
	{
		outError = L"Failed to checkout instance for writing.";
		return nullptr;
	}
	if (!instance->setObject(object) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the imported object.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setString(L"type", type_name(object));
	result->setBoolean(L"committed", true);
	return result;
}

}
