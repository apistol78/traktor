/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"

#include <string>

namespace traktor
{

class Transform;
class Vector2;
class Vector4;

}

namespace traktor::model
{

class Animation;
class IModelOperation;
class Joint;
class Material;
class Model;
class Polygon;
class Pose;
class Vertex;

}

namespace traktor::mcp
{

class Json;
class ModelSession;

/*! Resolve the working model referenced by a tool's "handle" argument.
 * \ingroup MCP
 *
 * Reads an integer "handle" member, looks it up in \a session, and returns the
 * model. Sets \a outError and returns null on a missing/unknown handle. The
 * resolved handle is written to \a outHandle (0 when unresolved).
 */
model::Model* resolveModel(ModelSession* session, const Json* arguments, int32_t& outHandle, std::wstring& outError);

// \name Vector <-> JSON helpers.
// \{

Ref< Json > vec3ToJson(const Vector4& v);		//!< [x,y,z]
Ref< Json > vec4ToJson(const Vector4& v);		//!< [x,y,z,w]
Ref< Json > vec2ToJson(const Vector2& v);		//!< [u,v]

bool jsonToVec3(const Json* arr, Vector4& out, float w = 1.0f);	//!< [x,y,z] -> (x,y,z,w)
bool jsonToVec4(const Json* arr, Vector4& out);					//!< [x,y,z,w]
bool jsonToVec2(const Json* arr, Vector2& out);					//!< [u,v]

Ref< Json > transformToJson(const Transform& transform);		//!< { translation:[x,y,z], rotation:[x,y,z,w] }
Transform transformFromJson(const Json* obj);					//!< reads translation:[x,y,z] and rotation:[x,y,z,w] (defaults: origin/identity)

// \}

// \name Model element -> JSON (read).
// \{

Ref< Json > materialToJson(const model::Material& material);
Ref< Json > jointToJson(const model::Joint& joint);
Ref< Json > animationToJson(const model::Animation* animation);
Ref< Json > polygonToJson(const model::Polygon& polygon);
Ref< Json > vertexToJson(const model::Vertex& vertex);

// \}

// \name JSON -> model element (write).
// \{

model::Material materialFromJson(const Json* obj);
model::Joint jointFromJson(const Json* obj);
model::Polygon polygonFromJson(const Json* obj);
model::Vertex vertexFromJson(const Json* obj);

/*! Build a Pose from { "jointTransforms": [ { translation, rotation }, ... ] }.
 * Array position is the joint id. Returns an empty pose for a missing/empty array. */
Ref< model::Pose > poseFromJson(const Json* obj);

// \}

/*! Decode a Model::Cf* clear-flag array (e.g. ["Polygons","Vertices"]) to a bitmask.
 * An empty/absent array yields Model::CfAll. Unknown names set \a outError. */
uint32_t clearFlagsFromJson(const Json* flagsArray, std::wstring& outError);

/*! Compact model overview: element counts and bounding box. */
Ref< Json > modelCountsToJson(model::Model* model);

/*! Full model summary: counts, bounding box, and the (small) material, joint,
 * animation, texcoord-channel and blend-target lists. Never dumps raw geometry. */
Ref< Json > modelInspectToJson(model::Model* model);

/*! Build a global model operation from a { "name": ..., <params> } spec.
 *
 * Covers every model::IModelOperation subclass. Operations that consume a second
 * model (Boolean, MergeModel) read a "modelHandle" resolved against \a session.
 * Returns null and sets \a outError on an unknown name or bad parameters.
 */
Ref< model::IModelOperation > createOperation(const Json* opSpec, ModelSession* session, std::wstring& outError);

}
