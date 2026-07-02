/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ModelGetElementsTool.h"

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "MCP/Server/Editor/ModelSession.h"
#include "MCP/Server/Editor/ModelToolSupport.h"
#include "MCP/Server/Json.h"
#include "Model/Model.h"

namespace traktor::mcp
{
namespace
{

const uint32_t c_defaultCount = 256;
const uint32_t c_maxCount = 4096;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelGetElementsTool", ModelGetElementsTool, IMcpTool)

ModelGetElementsTool::ModelGetElementsTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelGetElementsTool::getName() const
{
	return L"model_get_elements";
}

std::wstring ModelGetElementsTool::getDescription() const
{
	return L"Read a page of one of a model's raw element arrays. Args: \"handle\", \"kind\" (one of positions, normals, colors, texCoords, vertices, polygons, joints), optional \"offset\" (default 0) and \"count\" (default 256, capped at 4096). Returns { kind, total, offset, count, elements }. positions/normals are [x,y,z]; colors are [r,g,b,a]; texCoords are [u,v]; vertices/polygons/joints are objects (indices into the position/normal/... arrays). Page with offset+count for large meshes.";
}

Ref< Json > ModelGetElementsTool::getInputSchema() const
{
	Ref< Json > handle = Json::createObject();
	handle->setString(L"type", L"integer");
	handle->setString(L"description", L"Handle of an open model.");

	Ref< Json > kind = Json::createObject();
	kind->setString(L"type", L"string");
	kind->setString(L"description", L"positions | normals | colors | texCoords | vertices | polygons | joints");

	Ref< Json > offset = Json::createObject();
	offset->setString(L"type", L"integer");
	offset->setString(L"description", L"First element index to return (default 0).");

	Ref< Json > count = Json::createObject();
	count->setString(L"type", L"integer");
	count->setString(L"description", L"Number of elements to return (default 256, max 4096).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"handle", handle);
	properties->set(L"kind", kind);
	properties->set(L"offset", offset);
	properties->set(L"count", count);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"handle"));
	required->push(Json::createString(L"kind"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ModelGetElementsTool::invoke(const Json* arguments, std::wstring& outError)
{
	int32_t handle = 0;
	model::Model* model = resolveModel(m_session, arguments, handle, outError);
	if (!model)
		return nullptr;

	const std::wstring kind = arguments->getMember(L"kind") ? arguments->getMember(L"kind")->getString() : L"";
	if (kind.empty())
	{
		outError = L"Missing \"kind\".";
		return nullptr;
	}

	// Total count for the requested kind.
	uint32_t total = 0;
	if (kind == L"positions") total = model->getPositionCount();
	else if (kind == L"normals") total = model->getNormalCount();
	else if (kind == L"colors") total = model->getColorCount();
	else if (kind == L"texCoords") total = (uint32_t)model->getTexCoords().size();
	else if (kind == L"vertices") total = model->getVertexCount();
	else if (kind == L"polygons") total = model->getPolygonCount();
	else if (kind == L"joints") total = model->getJointCount();
	else
	{
		outError = L"Unknown \"kind\" \"" + kind + L"\". Valid: positions, normals, colors, texCoords, vertices, polygons, joints.";
		return nullptr;
	}

	const int64_t offsetArg = arguments->getMember(L"offset") ? arguments->getMember(L"offset")->getNumber() : 0;
	const uint32_t offset = offsetArg < 0 ? 0 : (uint32_t)offsetArg;
	uint32_t count = arguments->getMember(L"count") ? (uint32_t)arguments->getMember(L"count")->getNumber(c_defaultCount) : c_defaultCount;
	if (count > c_maxCount)
		count = c_maxCount;

	const uint32_t begin = offset < total ? offset : total;
	const uint32_t end = (begin + count < total) ? begin + count : total;

	Ref< Json > elements = Json::createArray();
	for (uint32_t i = begin; i < end; ++i)
	{
		if (kind == L"positions") elements->push(vec3ToJson(model->getPosition(i)));
		else if (kind == L"normals") elements->push(vec3ToJson(model->getNormal(i)));
		else if (kind == L"colors") elements->push(vec4ToJson(model->getColor(i)));
		else if (kind == L"texCoords") elements->push(vec2ToJson(model->getTexCoord(i)));
		else if (kind == L"vertices") elements->push(vertexToJson(model->getVertex(i)));
		else if (kind == L"polygons") elements->push(polygonToJson(model->getPolygon(i)));
		else if (kind == L"joints") elements->push(jointToJson(model->getJoint(i)));
	}

	Ref< Json > result = Json::createObject();
	result->set(L"handle", Json::createNumber((int64_t)handle));
	result->setString(L"kind", kind);
	result->set(L"total", Json::createNumber((int64_t)total));
	result->set(L"offset", Json::createNumber((int64_t)begin));
	result->set(L"count", Json::createNumber((int64_t)elements->size()));
	result->set(L"elements", elements);
	return result;
}

}
