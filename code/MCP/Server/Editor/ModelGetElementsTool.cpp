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
#include "Model/Animation.h"
#include "Model/Joint.h"
#include "Model/Model.h"
#include "Model/Pose.h"

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
	return L"Read a page of one of a model's raw element arrays. Args: \"handle\", \"kind\" (one of positions, normals, colors, texCoords, vertices, polygons, joints, pose), optional \"offset\" (default 0) and \"count\" (default 256, capped at 4096). Returns { kind, total, offset, count, elements }. positions/normals are [x,y,z]; colors are [r,g,b,a]; texCoords are [u,v]; vertices/polygons/joints are objects (indices into the position/normal/... arrays). Page with offset+count for large meshes.\n"
		L"kind \"pose\" reads a keyframe's joint transforms and additionally requires \"animation\" and \"keyFrame\" (indices); each element is { joint, name, translation:[x,y,z], rotation:[x,y,z,w] } for joint id offset+i. \"total\" is the pose's stored joint-transform count (may be fewer than the skeleton's jointCount; missing joints keep their bind transform). The result also carries \"time\" and \"skeletonJointCount\". Write poses with model_edit (addAnimation/addKeyFrame/setKeyFramePose/setPoseJoint).";
}

Ref< Json > ModelGetElementsTool::getInputSchema() const
{
	Ref< Json > handle = Json::createObject();
	handle->setString(L"type", L"integer");
	handle->setString(L"description", L"Handle of an open model.");

	Ref< Json > kind = Json::createObject();
	kind->setString(L"type", L"string");
	kind->setString(L"description", L"positions | normals | colors | texCoords | vertices | polygons | joints | pose");

	Ref< Json > offset = Json::createObject();
	offset->setString(L"type", L"integer");
	offset->setString(L"description", L"First element index to return (default 0).");

	Ref< Json > count = Json::createObject();
	count->setString(L"type", L"integer");
	count->setString(L"description", L"Number of elements to return (default 256, max 4096).");

	Ref< Json > animation = Json::createObject();
	animation->setString(L"type", L"integer");
	animation->setString(L"description", L"Animation index (required when kind=pose).");

	Ref< Json > keyFrame = Json::createObject();
	keyFrame->setString(L"type", L"integer");
	keyFrame->setString(L"description", L"Key frame index within the animation (required when kind=pose).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"handle", handle);
	properties->set(L"kind", kind);
	properties->set(L"offset", offset);
	properties->set(L"count", count);
	properties->set(L"animation", animation);
	properties->set(L"keyFrame", keyFrame);

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

	const int64_t offsetArg = arguments->getMember(L"offset") ? arguments->getMember(L"offset")->getNumber() : 0;
	const uint32_t offset = offsetArg < 0 ? 0 : (uint32_t)offsetArg;
	uint32_t count = arguments->getMember(L"count") ? (uint32_t)arguments->getMember(L"count")->getNumber(c_defaultCount) : c_defaultCount;
	if (count > c_maxCount)
		count = c_maxCount;

	// Poses are addressed by (animation, keyFrame) and page over the pose's stored
	// joint transforms, so they are read separately from the flat model arrays.
	if (kind == L"pose")
	{
		const Json* animArg = arguments->getMember(L"animation");
		const Json* kfArg = arguments->getMember(L"keyFrame");
		if (!animArg || !animArg->isNumber() || !kfArg || !kfArg->isNumber())
		{
			outError = L"kind \"pose\" requires integer \"animation\" and \"keyFrame\".";
			return nullptr;
		}
		const int64_t animIndex = animArg->getNumber();
		if (animIndex < 0 || animIndex >= (int64_t)model->getAnimationCount())
		{
			outError = L"\"animation\" out of range (model has " + std::to_wstring(model->getAnimationCount()) + L").";
			return nullptr;
		}
		const model::Animation* animation = model->getAnimation((uint32_t)animIndex);
		const int64_t keyFrame = kfArg->getNumber();
		if (keyFrame < 0 || keyFrame >= (int64_t)animation->getKeyFrameCount())
		{
			outError = L"\"keyFrame\" out of range (animation has " + std::to_wstring(animation->getKeyFrameCount()) + L").";
			return nullptr;
		}
		const model::Pose* pose = animation->getKeyFramePose((uint32_t)keyFrame);

		const uint32_t total = pose ? pose->getJointTransformCount() : 0;
		const uint32_t begin = offset < total ? offset : total;
		const uint32_t end = (begin + count < total) ? begin + count : total;

		Ref< Json > elements = Json::createArray();
		for (uint32_t i = begin; i < end; ++i)
		{
			Ref< Json > e = transformToJson(pose->getJointTransform(i));
			e->set(L"joint", Json::createNumber((int64_t)i));
			if (i < model->getJointCount())
				e->setString(L"name", model->getJoint(i).getName());
			elements->push(e);
		}

		Ref< Json > result = Json::createObject();
		result->set(L"handle", Json::createNumber((int64_t)handle));
		result->setString(L"kind", kind);
		result->set(L"animation", Json::createNumber(animIndex));
		result->set(L"keyFrame", Json::createNumber(keyFrame));
		result->set(L"time", Json::createReal(animation->getKeyFrameTime((uint32_t)keyFrame)));
		result->set(L"skeletonJointCount", Json::createNumber((int64_t)model->getJointCount()));
		result->set(L"total", Json::createNumber((int64_t)total));
		result->set(L"offset", Json::createNumber((int64_t)begin));
		result->set(L"count", Json::createNumber((int64_t)elements->size()));
		result->set(L"elements", elements);
		return result;
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
		outError = L"Unknown \"kind\" \"" + kind + L"\". Valid: positions, normals, colors, texCoords, vertices, polygons, joints, pose.";
		return nullptr;
	}

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
