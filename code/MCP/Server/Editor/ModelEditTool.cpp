/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ModelEditTool.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "MCP/Server/Editor/ModelSession.h"
#include "MCP/Server/Editor/ModelToolSupport.h"
#include "MCP/Server/Json.h"
#include "Model/Animation.h"
#include "Model/Joint.h"
#include "Model/Material.h"
#include "Model/Model.h"
#include "Model/Polygon.h"
#include "Model/Pose.h"
#include "Model/Vertex.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelEditTool", ModelEditTool, IMcpTool)

ModelEditTool::ModelEditTool(editor::IEditor* editor, ModelSession* session)
	: m_editor(editor)
	, m_session(session)
{
}

std::wstring ModelEditTool::getName() const
{
	return L"model_edit";
}

std::wstring ModelEditTool::getDescription() const
{
	return L"Apply a batch of fine-grained mutations to an open model (by \"handle\"). \"edits\" is an array of { \"op\", ... } applied in order. Ops:\n"
		L"- add: addPosition/addUniquePosition {value:[x,y,z]}, addNormal {value:[x,y,z]}, addColor {value:[r,g,b,a]}, addTexCoord {value:[u,v]}, addTexCoordChannel {name}, addBlendTarget {name}, addMaterial {name,color,diffuseTerm,specularTerm,roughness,metalness,transparency,emissive,reflective,doubleSided,blendOperator,maps}, addVertex {position,normal,color,tangent,binormal,texCoords:[idx...],jointInfluences:[{joint,weight}]}, addPolygon {material,normal,smoothGroup,vertices:[vertexIdx...]}, addJoint {name,parent,translation:[x,y,z],rotation:[x,y,z,w],length}. Each returns its new \"index\".\n"
		L"- set in place: setPosition {index,value:[x,y,z]}, setVertex {index,...}, setPolygon {index,...}, setJointRotation {index,value:[x,y,z,w]}, setBlendTargetPosition {blendTarget,position,value:[x,y,z]}.\n"
		L"- bulk replace (enables removal by omitting elements): setPositions/setNormals {values:[[x,y,z]...]}, setColors {values:[[r,g,b,a]...]}, setTexCoords {values:[[u,v]...]}, setVertices/setPolygons/setJoints/setMaterials {values:[{...}...]}.\n"
		L"- remove: removeMaterial {index}, removePolygons {indices:[...]}, clear {flags:[Materials,Vertices,Polygons,Positions,Colors,Normals,TexCoords,Joints] (omit for All)}.\n"
		L"- animation/pose: addAnimation {name} (returns \"index\"), setAnimationName {animation,name}, addKeyFrame {animation,time,pose} (appends a key frame, returns its \"index\"; keep times ascending), setKeyFrameTime {animation,keyFrame,time}, setKeyFramePose {animation,keyFrame,pose}, setPoseJoint {animation,keyFrame,joint,translation,rotation} (edits a single joint in a key frame's pose, translation/rotation optional). A \"pose\" is { \"jointTransforms\": [ { \"translation\":[x,y,z], \"rotation\":[x,y,z,w] }, ... ] } where array position is the joint id; joints beyond the array keep their skeleton bind transform. Read poses back with model_get_elements kind=pose. The Model API has no removeAnimation/removeKeyFrame.\n"
		L"Note: the Model API has no per-vertex/per-polygon delete beyond these; remove elements with removePolygons, removeMaterial, clear, bulk set (send the survivors), or the cleanup operations in model_apply_operation. Edits apply in order; on error, edits before the failing one have already been applied. Returns { handle, results, summary }.";
}

Ref< Json > ModelEditTool::getInputSchema() const
{
	Ref< Json > handle = Json::createObject();
	handle->setString(L"type", L"integer");
	handle->setString(L"description", L"Handle of an open model.");

	Ref< Json > edits = Json::createObject();
	edits->setString(L"type", L"array");
	edits->setString(L"description", L"Ordered list of { op, ... } mutations (see tool description).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"handle", handle);
	properties->set(L"edits", edits);

	Ref< Json > required = Json::createArray();
	required->push(Json::createString(L"handle"));
	required->push(Json::createString(L"edits"));

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", required);
	return schema;
}

Ref< Json > ModelEditTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< Json > coerced = coerceStructuredArguments(arguments);
	arguments = coerced;

	int32_t handle = 0;
	model::Model* model = resolveModel(m_session, arguments, handle, outError);
	if (!model)
		return nullptr;

	const Json* edits = arguments->getMember(L"edits");
	if (!edits || !edits->isArray())
	{
		outError = L"\"edits\" must be an array of { op, ... } objects.";
		return nullptr;
	}

	// Bulk vector builders (return false on a malformed element).
	auto buildVec = [](const Json* values, bool rgba, float w, AlignedVector< Vector4 >& out) -> bool {
		if (!values || !values->isArray())
			return false;
		out.resize(values->size());
		for (uint32_t i = 0; i < values->size(); ++i)
		{
			Vector4 v;
			const bool ok = rgba ? jsonToVec4(values->at(i), v) : jsonToVec3(values->at(i), v, w);
			if (!ok)
				return false;
			out[i] = v;
		}
		return true;
	};

	Ref< Json > results = Json::createArray();

	for (uint32_t i = 0; i < edits->size(); ++i)
	{
		const Json* edit = edits->at(i);
		if (!edit || !edit->isObject() || !edit->getMember(L"op"))
		{
			outError = L"edits[" + std::to_wstring(i) + L"] must be an object with an \"op\".";
			return nullptr;
		}
		const std::wstring op = edit->getMember(L"op")->getString();
		const Json* valueJson = edit->getMember(L"value");

		Ref< Json > r = Json::createObject();
		r->setString(L"op", op);

		auto fail = [&](const std::wstring& msg) {
			outError = L"edits[" + std::to_wstring(i) + L"] (" + op + L"): " + msg;
		};
		auto memberIndex = [&](const wchar_t* key, uint32_t& out) -> bool {
			const Json* m = edit->getMember(key);
			if (!m || !m->isNumber() || m->getNumber() < 0)
				return false;
			out = (uint32_t)m->getNumber();
			return true;
		};

		if (op == L"addPosition" || op == L"addUniquePosition")
		{
			Vector4 v;
			if (!jsonToVec3(valueJson, v, 1.0f)) { fail(L"\"value\" must be [x,y,z]."); return nullptr; }
			const uint32_t index = (op == L"addUniquePosition") ? model->addUniquePosition(v) : model->addPosition(v);
			r->set(L"index", Json::createNumber((int64_t)index));
		}
		else if (op == L"addNormal")
		{
			Vector4 v;
			if (!jsonToVec3(valueJson, v, 0.0f)) { fail(L"\"value\" must be [x,y,z]."); return nullptr; }
			r->set(L"index", Json::createNumber((int64_t)model->addNormal(v)));
		}
		else if (op == L"addColor")
		{
			Vector4 v;
			if (!jsonToVec4(valueJson, v)) { fail(L"\"value\" must be [r,g,b,a]."); return nullptr; }
			r->set(L"index", Json::createNumber((int64_t)model->addColor(v)));
		}
		else if (op == L"addTexCoord")
		{
			Vector2 v;
			if (!jsonToVec2(valueJson, v)) { fail(L"\"value\" must be [u,v]."); return nullptr; }
			r->set(L"index", Json::createNumber((int64_t)model->addTexCoord(v)));
		}
		else if (op == L"addTexCoordChannel")
		{
			const std::wstring name = edit->getMember(L"name") ? edit->getMember(L"name")->getString() : L"";
			if (name.empty()) { fail(L"\"name\" is required."); return nullptr; }
			r->set(L"index", Json::createNumber((int64_t)model->addUniqueTexCoordChannel(name)));
		}
		else if (op == L"addBlendTarget")
		{
			const std::wstring name = edit->getMember(L"name") ? edit->getMember(L"name")->getString() : L"";
			if (name.empty()) { fail(L"\"name\" is required."); return nullptr; }
			r->set(L"index", Json::createNumber((int64_t)model->addBlendTarget(name)));
		}
		else if (op == L"addMaterial")
			r->set(L"index", Json::createNumber((int64_t)model->addMaterial(materialFromJson(edit))));
		else if (op == L"addVertex")
			r->set(L"index", Json::createNumber((int64_t)model->addVertex(vertexFromJson(edit))));
		else if (op == L"addPolygon")
			r->set(L"index", Json::createNumber((int64_t)model->addPolygon(polygonFromJson(edit))));
		else if (op == L"addJoint")
			r->set(L"index", Json::createNumber((int64_t)model->addJoint(jointFromJson(edit))));
		else if (op == L"setPosition")
		{
			uint32_t index;
			Vector4 v;
			if (!memberIndex(L"index", index) || index >= model->getPositionCount()) { fail(L"missing/out-of-range \"index\"."); return nullptr; }
			if (!jsonToVec3(valueJson, v, 1.0f)) { fail(L"\"value\" must be [x,y,z]."); return nullptr; }
			model->setPosition(index, v);
		}
		else if (op == L"setVertex")
		{
			uint32_t index;
			if (!memberIndex(L"index", index) || index >= model->getVertexCount()) { fail(L"missing/out-of-range \"index\"."); return nullptr; }
			model->setVertex(index, vertexFromJson(edit));
		}
		else if (op == L"setPolygon")
		{
			uint32_t index;
			if (!memberIndex(L"index", index) || index >= model->getPolygonCount()) { fail(L"missing/out-of-range \"index\"."); return nullptr; }
			model->setPolygon(index, polygonFromJson(edit));
		}
		else if (op == L"setJointRotation")
		{
			uint32_t index;
			Vector4 v;
			if (!memberIndex(L"index", index) || index >= model->getJointCount()) { fail(L"missing/out-of-range \"index\"."); return nullptr; }
			if (!jsonToVec4(valueJson, v)) { fail(L"\"value\" must be a quaternion [x,y,z,w]."); return nullptr; }
			model->setJointRotation(index, Quaternion(v.x(), v.y(), v.z(), v.w()));
		}
		else if (op == L"setBlendTargetPosition")
		{
			uint32_t blendTarget, position;
			Vector4 v;
			if (!memberIndex(L"blendTarget", blendTarget) || blendTarget >= model->getBlendTargetCount()) { fail(L"missing/out-of-range \"blendTarget\"."); return nullptr; }
			if (!memberIndex(L"position", position)) { fail(L"missing \"position\"."); return nullptr; }
			if (!jsonToVec3(valueJson, v, 1.0f)) { fail(L"\"value\" must be [x,y,z]."); return nullptr; }
			model->setBlendTargetPosition(blendTarget, position, v);
		}
		else if (op == L"setPositions" || op == L"setNormals" || op == L"setColors")
		{
			AlignedVector< Vector4 > values;
			const bool rgba = (op == L"setColors");
			const float w = (op == L"setPositions") ? 1.0f : 0.0f;
			if (!buildVec(edit->getMember(L"values"), rgba, w, values)) { fail(L"\"values\" must be an array of vectors."); return nullptr; }
			if (op == L"setPositions") model->setPositions(values);
			else if (op == L"setNormals") model->setNormals(values);
			else model->setColors(values);
			r->set(L"count", Json::createNumber((int64_t)values.size()));
		}
		else if (op == L"setTexCoords")
		{
			const Json* values = edit->getMember(L"values");
			if (!values || !values->isArray()) { fail(L"\"values\" must be an array of [u,v]."); return nullptr; }
			AlignedVector< Vector2 > texCoords;
			texCoords.resize(values->size());
			for (uint32_t k = 0; k < values->size(); ++k)
			{
				if (!jsonToVec2(values->at(k), texCoords[k])) { fail(L"\"values\" entries must be [u,v]."); return nullptr; }
			}
			model->setTexCoords(texCoords);
			r->set(L"count", Json::createNumber((int64_t)texCoords.size()));
		}
		else if (op == L"setVertices" || op == L"setPolygons" || op == L"setJoints" || op == L"setMaterials")
		{
			const Json* values = edit->getMember(L"values");
			if (!values || !values->isArray()) { fail(L"\"values\" must be an array of objects."); return nullptr; }
			if (op == L"setVertices")
			{
				AlignedVector< model::Vertex > v(values->size());
				for (uint32_t k = 0; k < values->size(); ++k) v[k] = vertexFromJson(values->at(k));
				model->setVertices(v);
			}
			else if (op == L"setPolygons")
			{
				AlignedVector< model::Polygon > v(values->size());
				for (uint32_t k = 0; k < values->size(); ++k) v[k] = polygonFromJson(values->at(k));
				model->setPolygons(v);
			}
			else if (op == L"setJoints")
			{
				AlignedVector< model::Joint > v(values->size());
				for (uint32_t k = 0; k < values->size(); ++k) v[k] = jointFromJson(values->at(k));
				model->setJoints(v);
			}
			else
			{
				AlignedVector< model::Material > v(values->size());
				for (uint32_t k = 0; k < values->size(); ++k) v[k] = materialFromJson(values->at(k));
				model->setMaterials(v);
			}
			r->set(L"count", Json::createNumber((int64_t)values->size()));
		}
		else if (op == L"removeMaterial")
		{
			uint32_t index;
			if (!memberIndex(L"index", index)) { fail(L"missing \"index\"."); return nullptr; }
			r->setBoolean(L"removed", model->removeMaterial(index));
		}
		else if (op == L"removePolygons")
		{
			const Json* indices = edit->getMember(L"indices");
			if (!indices || !indices->isArray()) { fail(L"\"indices\" must be an array."); return nullptr; }
			AlignedVector< bool > drop(model->getPolygonCount(), false);
			for (uint32_t k = 0; k < indices->size(); ++k)
			{
				const int64_t idx = indices->at(k)->getNumber(-1);
				if (idx >= 0 && idx < (int64_t)drop.size())
					drop[(uint32_t)idx] = true;
			}
			const AlignedVector< model::Polygon >& src = model->getPolygons();
			AlignedVector< model::Polygon > kept;
			kept.reserve(src.size());
			for (uint32_t k = 0; k < src.size(); ++k)
				if (!drop[k])
					kept.push_back(src[k]);
			const uint32_t removed = (uint32_t)(src.size() - kept.size());
			model->setPolygons(kept);
			r->set(L"removed", Json::createNumber((int64_t)removed));
		}
		else if (op == L"clear")
		{
			const uint32_t flags = clearFlagsFromJson(edit->getMember(L"flags"), outError);
			if (!outError.empty()) { outError = L"edits[" + std::to_wstring(i) + L"]: " + outError; return nullptr; }
			model->clear(flags);
		}
		else if (op == L"addAnimation")
		{
			Ref< model::Animation > animation = new model::Animation();
			if (edit->getMember(L"name"))
				animation->setName(edit->getMember(L"name")->getString());
			r->set(L"index", Json::createNumber((int64_t)model->addAnimation(animation)));
		}
		else if (op == L"setAnimationName" || op == L"addKeyFrame" || op == L"setKeyFrameTime" || op == L"setKeyFramePose" || op == L"setPoseJoint")
		{
			// All of these ops address an existing animation. getAnimations() indexing
			// yields a mutable Animation (const RefArray only pins the array, not its
			// elements), which addAnimation/getAnimation-const alone would not give us.
			uint32_t animIndex;
			if (!memberIndex(L"animation", animIndex) || animIndex >= model->getAnimationCount()) { fail(L"missing/out-of-range \"animation\"."); return nullptr; }
			model::Animation* animation = model->getAnimations()[animIndex];

			if (op == L"setAnimationName")
			{
				animation->setName(edit->getMember(L"name") ? edit->getMember(L"name")->getString() : L"");
			}
			else if (op == L"addKeyFrame")
			{
				// pose is optional; an absent/empty pose yields an empty pose (all joints at bind).
				const Json* timeJson = edit->getMember(L"time");
				animation->insertKeyFrame(timeJson ? (float)timeJson->getReal() : 0.0f, poseFromJson(edit->getMember(L"pose")));
				r->set(L"index", Json::createNumber((int64_t)(animation->getKeyFrameCount() - 1)));
			}
			else
			{
				// Remaining ops address an existing key frame within the animation.
				uint32_t keyFrame;
				if (!memberIndex(L"keyFrame", keyFrame) || keyFrame >= animation->getKeyFrameCount()) { fail(L"missing/out-of-range \"keyFrame\"."); return nullptr; }

				if (op == L"setKeyFrameTime")
				{
					const Json* timeJson = edit->getMember(L"time");
					if (!timeJson) { fail(L"missing \"time\"."); return nullptr; }
					animation->setKeyFrameTime(keyFrame, (float)timeJson->getReal());
				}
				else if (op == L"setKeyFramePose")
				{
					animation->setKeyFramePose(keyFrame, poseFromJson(edit->getMember(L"pose")));
				}
				else // setPoseJoint
				{
					uint32_t joint;
					if (!memberIndex(L"joint", joint)) { fail(L"missing \"joint\"."); return nullptr; }

					// A key frame pose is immutable (Ref< const Pose >), so clone it,
					// override one joint, and replace. Gaps opened up past the pose's
					// current stored count are filled with the skeleton bind transform
					// so those joints keep their bind relationship rather than collapsing
					// to identity.
					const model::Pose* current = animation->getKeyFramePose(keyFrame);
					const uint32_t existing = current ? current->getJointTransformCount() : 0;

					Ref< model::Pose > pose = new model::Pose();
					for (uint32_t k = 0; k < existing; ++k)
						pose->setJointTransform(k, current->getJointTransform(k));
					for (uint32_t k = existing; k < joint; ++k)
						pose->setJointTransform(k, k < model->getJointCount() ? model->getJoint(k).getTransform() : Transform::identity());

					// Start from the joint's current pose transform (bind if beyond the
					// stored count) and apply whichever of translation/rotation was given.
					Transform base = (joint < existing)
						? current->getJointTransform(joint)
						: (joint < model->getJointCount() ? model->getJoint(joint).getTransform() : Transform::identity());
					Vector4 t = base.translation();
					Quaternion q = base.rotation();
					Vector4 tv;
					if (jsonToVec3(edit->getMember(L"translation"), tv, 1.0f)) t = tv;
					Vector4 rv;
					if (jsonToVec4(edit->getMember(L"rotation"), rv)) q = Quaternion(rv.x(), rv.y(), rv.z(), rv.w());
					pose->setJointTransform(joint, Transform(t, q));

					animation->setKeyFramePose(keyFrame, pose);
					r->set(L"jointTransformCount", Json::createNumber((int64_t)pose->getJointTransformCount()));
				}
			}
		}
		else
		{
			fail(L"unknown op.");
			return nullptr;
		}

		results->push(r);
	}

	Ref< Json > result = Json::createObject();
	result->set(L"handle", Json::createNumber((int64_t)handle));
	result->set(L"results", results);
	result->set(L"summary", modelCountsToJson(model));
	return result;
}

}
