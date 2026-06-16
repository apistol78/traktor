/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Guid.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Core/Misc/Base64.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/ImportTextureFromDataTool.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ImportTextureFromDataTool", ImportTextureFromDataTool, IMcpTool)

ImportTextureFromDataTool::ImportTextureFromDataTool(editor::IEditor* editor)
:	m_editor(editor)
{
}

std::wstring ImportTextureFromDataTool::getName() const
{
	return L"import_texture_from_data";
}

std::wstring ImportTextureFromDataTool::getDescription() const
{
	return L"Create a texture from in-memory image data. Decodes base64 image bytes (\"data\") and writes them to a source file under the asset path (\"fileName\", relative to Pipeline.AssetPath, e.g. \"Textures/Bricks.tga\"; the extension selects the format - tga, png, jpg, ...), then creates a traktor.render.TextureAsset instance at \"path\" referencing it. The asset still needs to be built (see build_asset) before it can be sampled at runtime.";
}

Ref< Json > ImportTextureFromDataTool::getInputSchema() const
{
	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path for the new TextureAsset instance (e.g. \"Textures/Bricks\").");

	Ref< Json > fileNameProperty = Json::createObject();
	fileNameProperty->setString(L"type", L"string");
	fileNameProperty->setString(L"description", L"Image file path to write, relative to the asset path (e.g. \"Textures/Bricks.tga\"). The extension selects the format.");

	Ref< Json > dataProperty = Json::createObject();
	dataProperty->setString(L"type", L"string");
	dataProperty->setString(L"description", L"Base64-encoded contents of the image file.");

	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Optional explicit guid for the new instance (otherwise one is generated; an existing instance at the path keeps its guid).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"path", pathProperty);
	properties->set(L"fileName", fileNameProperty);
	properties->set(L"data", dataProperty);
	properties->set(L"guid", guidProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"path"));
	required->push(Json::createString(L"fileName"));
	required->push(Json::createString(L"data"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ImportTextureFromDataTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	const std::wstring path = (arguments && arguments->getMember(L"path")) ? arguments->getMember(L"path")->getString() : L"";
	const std::wstring fileName = (arguments && arguments->getMember(L"fileName")) ? arguments->getMember(L"fileName")->getString() : L"";
	const std::wstring b64 = (arguments && arguments->getMember(L"data")) ? arguments->getMember(L"data")->getString() : L"";
	if (path.empty() || fileName.empty() || b64.empty())
	{
		outError = L"Missing \"path\", \"fileName\" and/or \"data\".";
		return nullptr;
	}

	const AlignedVector< uint8_t > bytes = Base64().decode(b64);
	if (bytes.empty())
	{
		outError = L"Failed to decode base64 \"data\" (decoded to zero bytes).";
		return nullptr;
	}

	Ref< const PropertyGroup > settings = m_editor->getSettings();
	const std::wstring assetPath = settings ? settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"") : L"";
	if (assetPath.empty())
	{
		outError = L"Pipeline.AssetPath is not configured; cannot resolve where to write the image.";
		return nullptr;
	}

	// Resolve "fileName" relative to the asset path, then to an absolute path.
	const Path absolutePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath), Path(fileName));

	if (!FileSystem::getInstance().makeAllDirectories(absolutePath.getPathOnly()))
	{
		outError = L"Failed to create directory: " + absolutePath.getPathOnly();
		return nullptr;
	}

	Ref< IStream > stream = FileSystem::getInstance().open(absolutePath, File::FmWrite);
	if (!stream)
	{
		outError = L"Failed to open file for writing: " + absolutePath.getPathName();
		return nullptr;
	}
	const int64_t written = stream->write(bytes.c_ptr(), (int64_t)bytes.size());
	stream->close();
	if (written != (int64_t)bytes.size())
	{
		outError = L"Failed to write the full image data to " + absolutePath.getPathName();
		return nullptr;
	}

	// Create the texture asset referencing the written file.
	Ref< render::TextureAsset > textureAsset = new render::TextureAsset();
	textureAsset->setFileName(Path(fileName));

	Guid guid;
	if (arguments->getMember(L"guid"))
	{
		const Guid g(arguments->getMember(L"guid")->getString());
		if (g.isValid())
			guid = g;
	}
	if (!guid.isValid())
		guid = Guid::create();

	Ref< db::Instance > instance = database->createInstance(path, db::CifReplaceExisting | db::CifKeepExistingGuid, &guid);
	if (!instance)
	{
		outError = L"Failed to create instance at \"" + path + L"\".";
		return nullptr;
	}
	if (!instance->setObject(textureAsset) || !instance->commit())
	{
		instance->revert();
		outError = L"Failed to commit the new texture asset.";
		return nullptr;
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", path);
	result->setString(L"fileName", fileName);
	result->setString(L"file", absolutePath.getPathName());
	result->set(L"bytesWritten", Json::createNumber((int64_t)written));
	result->setBoolean(L"committed", true);
	return result;
}

}
