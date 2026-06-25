/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "MCP/Server/Editor/McpToolSupport.h"
#include "Mesh/Editor/MeshAsset.h"

#include <string>

namespace traktor
{

class Aabb3;
class ISerializable;

}

namespace traktor::mcp
{

class Json;

/*! Read and downcast a mesh asset instance.
 *
 * The primary type is verified by name first, so the downcast is safe; a static
 * cast (rather than dynamic_type_cast) deliberately avoids a link-time
 * dependency on Mesh.Editor's RTTI symbol - only MeshAsset's inline accessors
 * are used. The deserialized object is returned through \a outObject to keep it
 * alive for the duration of the call.
 */
mesh::MeshAsset* loadMeshAsset(db::Instance* instance, Ref< ISerializable >& outObject, std::wstring& outError);

/*! Build a { name: guid } JSON object from a material map. */
Ref< Json > materialMapToJson(const SmallMap< std::wstring, Guid >& map);

/*! Name of a mesh type. */
std::wstring meshTypeName(mesh::MeshAsset::MeshType type);

/*! JSON Schema { guid, path } properties for a mesh asset tool. */
Ref< Json > meshTargetProperties();

/*! Human-readable description of where the asset origin sits within a box.
 *
 * e.g. "base, horizontally centered" (a piece that rests on the ground) or
 * "mid-height, horizontally offset". Useful for reasoning about how a kit piece
 * snaps to a grid. */
std::wstring pivotLabel(const Aabb3& bbox);

/*! Build a JSON description of a model-space bounding box: size (per axis),
 * min/max/center, and the pivot (origin position within the box + label). */
Ref< Json > boundingBoxToJson(const Aabb3& bbox);

}
