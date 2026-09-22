/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <map>
#include <string>

#include "Core/Containers/AlignedVector.h"
#include "Mesh/Editor/MeshPipelineTypes.h"
#include "Mesh/MeshResource.h"

namespace traktor::model
{

class Model;

}

namespace traktor::render
{

class Mesh;

}

namespace traktor::mesh
{

/*! Deform geometry; the deform parts of a mesh and the vertex index lists they dispatch over.
 * \ingroup Mesh
 */
struct DeformGeometry
{
	AlignedVector< MeshResource::DeformPart > parts;
	AlignedVector< uint32_t > indices;

	bool empty() const { return parts.empty(); }
};

/*! Build deform geometry of a mesh.
 *
 * Each material with a deform technique gets the list of vertices its polygons reference,
 * in both the raster and the ray tracing model, dispatched with that technique. The
 * remaining vertices form a part copying undeformed positions so the whole deform buffer
 * is written every frame. Empty when no material deforms.
 *
 * \param model Raster model.
 * \param rtModel Ray tracing model, or null when ray tracing is disabled.
 * \param rtSharedVertexCount Number of leading ray tracing model vertices identical to the raster model's.
 * \param totalVertexCount Total number of vertices in the vertex buffer; raster and appended ray tracing ones.
 */
void buildDeformGeometry(
	const model::Model* model,
	const model::Model* rtModel,
	uint32_t rtSharedVertexCount,
	uint32_t totalVertexCount,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	DeformGeometry& outGeometry);

/*! Size of the deform aux buffers to declare for the render mesh. */
uint32_t getDeformVerticesSize(uint32_t totalVertexCount);

uint32_t getDeformIndicesSize(const DeformGeometry& geometry);

/*! Write the deform source vertices and vertex index lists into the render mesh's aux buffers.
 *
 * The source carry every vertex in a fixed layout so the Deform compute technique can read
 * the attributes the surface needs; appended ray tracing vertices take theirs from the ray
 * tracing model.
 */
void writeDeformGeometry(
	render::Mesh* renderMesh,
	const model::Model* model,
	const model::Model* rtModel,
	uint32_t rtSharedVertexCount,
	const DeformGeometry& geometry);

}
