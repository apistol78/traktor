/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/ModelApplyOperationTool.h"

#include "MCP/Editor/McpToolSupport.h"
#include "MCP/Editor/ModelSession.h"
#include "MCP/Editor/ModelToolSupport.h"
#include "MCP/Editor/Json.h"
#include "Model/IModelOperation.h"
#include "Model/Model.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelApplyOperationTool", ModelApplyOperationTool, IMcpTool)

ModelApplyOperationTool::ModelApplyOperationTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelApplyOperationTool::getName() const
{
	return L"model_apply_operation";
}

std::wstring ModelApplyOperationTool::getDescription() const
{
	return L"Apply one or more global model operations (fast whole-model manipulations) to an open model (by \"handle\"). \"operations\" is an array of { \"name\", <params> } applied in order. Names and params: Triangulate; CalculateNormals {replaceExisting}; CalculateTangents {replaceExisting}; CalculateConvexHull; CleanDegenerate; CleanDuplicates {positionDistance}; Clear {flags:[...]}; CullDistantFaces {region:{min:[x,y,z],max:[x,y,z]}}; FlattenDoubleSided; MergeCoplanarAdjacents; MergeModel {modelHandle,transform,positionDistance} (merges another open model into this one); NormalizeTexCoords {channel,marginU,marginV,stepU,stepV}; Quantize {step (number or [x,y,z])}; Reduce {target (0..1)}; ScaleAlongNormal {distance}; SortCacheCoherency; SortProjectedArea {insideOut}; Transform {translate:[x,y,z],rotate:[x,y,z,w],scale (number or [x,y,z])} or {matrix:[16]}; Unweld; UnwrapUV {channel,textureSize}; BakeVertexColors; Boolean {modelHandleA,modelHandleB,operation:union|intersection|difference,transformA,transformB} (writes the result into THIS handle - open a blank one). Returns { handle, before, applied, after }.";
}

Ref< Json > ModelApplyOperationTool::getInputSchema() const
{
	Ref< Json > handle = Json::createObject();
	handle->setString(L"type", L"integer");
	handle->setString(L"description", L"Handle of an open model.");

	Ref< Json > operations = Json::createObject();
	operations->setString(L"type", L"array");
	operations->setString(L"description", L"Ordered list of { name, <params> } operations (see tool description).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"handle", handle);
	properties->set(L"operations", operations);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"handle"));
	required->push(Json::createString(L"operations"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ModelApplyOperationTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< Json > coerced = coerceStructuredArguments(arguments);
	arguments = coerced;

	int32_t handle = 0;
	model::Model* model = resolveModel(m_session, arguments, handle, outError);
	if (!model)
		return nullptr;

	const Json* operations = arguments->getMember(L"operations");
	if (!operations || !operations->isArray() || operations->size() == 0)
	{
		outError = L"\"operations\" must be a non-empty array of { name, <params> } objects.";
		return nullptr;
	}

	Ref< Json > before = modelCountsToJson(model);

	Ref< Json > applied = Json::createArray();
	for (uint32_t i = 0; i < operations->size(); ++i)
	{
		Ref< model::IModelOperation > operation = createOperation(operations->at(i), m_session, outError);
		if (!operation)
		{
			outError = L"operations[" + std::to_wstring(i) + L"]: " + outError;
			return nullptr;
		}

		const bool ok = model->apply(*operation);

		Ref< Json > entry = Json::createObject();
		entry->setString(L"name", operations->at(i)->getMember(L"name")->getString());
		entry->setBoolean(L"applied", ok);
		applied->push(entry);
	}

	Ref< Json > result = Json::createObject();
	result->set(L"handle", Json::createNumber((int64_t)handle));
	result->set(L"before", before);
	result->set(L"applied", applied);
	result->set(L"after", modelCountsToJson(model));
	return result;
}

}
