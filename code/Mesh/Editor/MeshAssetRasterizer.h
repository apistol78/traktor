/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Aabb3;

}

namespace traktor::drawing
{

class Image;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mesh
{

class MeshAsset;

class T_DLLCLASS MeshAssetRasterizer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Render the asset using its authored preview angle (turntable about Y). */
	bool generate(const editor::IEditor* editor, const MeshAsset* asset, drawing::Image* outImage) const;

	/*! Render the asset from an explicit camera orientation.
	 *
	 * \param yaw Rotation about the vertical (Y) axis, radians.
	 * \param pitch Downward tilt about the X axis, radians.
	 *
	 * Frames on the model's largest axis so it stays in view at any angle.
	 * Used to produce multi-view previews (front/side/top/three-quarter).
	 */
	bool generate(const editor::IEditor* editor, const MeshAsset* asset, float yaw, float pitch, drawing::Image* outImage) const;

	/*! Read the model-space bounding box of the asset's source mesh.
	 *
	 * Loads only the geometry (no triangulation or material binding), so it is
	 * cheaper than a render. Returns false if the model file cannot be read or
	 * the mesh is empty.
	 */
	bool getBoundingBox(const editor::IEditor* editor, const MeshAsset* asset, Aabb3& outBoundingBox) const;
};

}
