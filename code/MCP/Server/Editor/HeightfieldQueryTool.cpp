/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "MCP/Server/Json.h"
#include "MCP/Server/Editor/HeightfieldQueryTool.h"
#include "MCP/Server/Editor/McpToolSupport.h"

namespace traktor::mcp
{
	namespace
{

const wchar_t* const c_heightfieldAssetTypeName = L"traktor.hf.HeightfieldAsset";

Ref< Json > vec3ToJson(const Vector4& v)
{
	Ref< Json > arr = Json::createArray();
	arr->push(Json::createReal((double)v.x()));
	arr->push(Json::createReal((double)v.y()));
	arr->push(Json::createReal((double)v.z()));
	return arr;
}

/*! Read x/z from a query point, accepting either { "x":, "z": } or [x, z]. */
bool readPoint(const Json* point, float& outX, float& outZ)
{
	if (!point)
		return false;
	if (point->isArray())
	{
		if (point->size() < 2)
			return false;
		outX = (float)point->at(0)->getReal();
		outZ = (float)point->at(1)->getReal();
		return true;
	}
	if (point->isObject())
	{
		const Json* x = point->getMember(L"x");
		const Json* z = point->getMember(L"z");
		if (!x || !z)
			return false;
		outX = (float)x->getReal();
		outZ = (float)z->getReal();
		return true;
	}
	return false;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.HeightfieldQueryTool", HeightfieldQueryTool, IMcpTool)

HeightfieldQueryTool::HeightfieldQueryTool(editor::IEditor* editor)
:	m_editor(editor)
{
}

std::wstring HeightfieldQueryTool::getName() const
{
	return L"query_heightfield";
}

std::wstring HeightfieldQueryTool::getDescription() const
{
	return L"Sample world heights from a traktor.hf.HeightfieldAsset. Given world XZ coordinates, returns the interpolated terrain height (world Y), the cut flag (a hole in the terrain) and the surface normal at each point. Also returns the heightfield's worldExtent and grid size. Use this to place objects on the ground or reason about terrain elevation. World XZ are centered on the origin: valid range is [-worldExtent/2, +worldExtent/2] on X and Z.";
}

Ref< Json > HeightfieldQueryTool::getInputSchema() const
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the heightfield asset instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the heightfield asset instance (alternative to guid).");

	// points: array of either { "x":, "z": } objects or [x, z] pairs.
	Ref< Json > pointsItems = Json::createObject();
	pointsItems->setString(L"type", L"object");

	Ref< Json > pointsProperty = Json::createObject();
	pointsProperty->setString(L"type", L"array");
	pointsProperty->setString(L"description", L"World XZ coordinates to sample, each as { \"x\": <number>, \"z\": <number> } (or [x, z]).");
	pointsProperty->set(L"items", pointsItems);

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);
	properties->set(L"points", pointsProperty);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"points"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > HeightfieldQueryTool::invoke(const Json* arguments, std::wstring& outError)
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
			outError = L"No heightfield asset instance found for the given guid/path.";
		return nullptr;
	}

	if (instance->getPrimaryTypeName() != c_heightfieldAssetTypeName)
	{
		outError = L"Instance is not a HeightfieldAsset (primary type: " + instance->getPrimaryTypeName() + L").";
		return nullptr;
	}

	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		outError = L"Failed to read heightfield asset object.";
		return nullptr;
	}
	hf::HeightfieldAsset* asset = static_cast< hf::HeightfieldAsset* >(object.ptr());

	// Read and parse the height data channel (mirrors HeightfieldAssetEditor).
	Ref< IStream > sourceData = instance->readData(L"Data");
	if (!sourceData)
	{
		outError = L"Unable to open heightfield \"Data\" channel.";
		return nullptr;
	}

	Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
		sourceData,
		asset->getWorldExtent()
	);
	sourceData->close();
	sourceData = nullptr;

	if (!heightfield)
	{
		outError = L"Failed to read heightfield heights.";
		return nullptr;
	}

	// "points" is normally a JSON array, but some MCP clients deliver array
	// arguments as a JSON-encoded string; decode that case here. (We only coerce
	// this member - coercing the whole argument object would misread a
	// brace-wrapped guid like "{...}" as a JSON object.)
	const Json* points = arguments ? arguments->getMember(L"points") : nullptr;
	Ref< Json > pointsDecoded;
	if (points && points->isString())
	{
		pointsDecoded = Json::parse(points->getString());
		points = pointsDecoded;
	}
	if (!points || !points->isArray())
	{
		outError = L"Missing \"points\"; expected an array of { x, z } world coordinates.";
		return nullptr;
	}

	Ref< Json > samples = Json::createArray();
	for (uint32_t i = 0; i < points->size(); ++i)
	{
		float worldX, worldZ;
		if (!readPoint(points->at(i), worldX, worldZ))
		{
			outError = L"Point at index " + std::to_wstring(i) + L" is not a valid { x, z } coordinate.";
			return nullptr;
		}

		const float height = heightfield->getWorldHeight(worldX, worldZ);
		const bool cut = heightfield->getWorldCut(worldX, worldZ);

		float gridX, gridZ;
		heightfield->worldToGrid(worldX, worldZ, gridX, gridZ);
		const Vector4 normal = heightfield->normalAt(gridX, gridZ);

		Ref< Json > sample = Json::createObject();
		sample->set(L"x", Json::createReal((double)worldX));
		sample->set(L"z", Json::createReal((double)worldZ));
		sample->set(L"height", Json::createReal((double)height));
		sample->setBoolean(L"cut", cut);
		sample->set(L"normal", vec3ToJson(normal));
		samples->push(sample);
	}

	Ref< Json > result = Json::createObject();
	result->setString(L"guid", instance->getGuid().format());
	result->setString(L"path", instance->getPath());
	result->setString(L"name", instance->getName());
	result->set(L"worldExtent", vec3ToJson(asset->getWorldExtent()));
	result->setNumber(L"size", heightfield->getSize());
	result->set(L"samples", samples);
	return result;
}

}
